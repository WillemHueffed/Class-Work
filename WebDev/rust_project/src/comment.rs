use crate::jwt::Auth;

const DB_NAME: &str = "WebDev";
const COLL_NAME: &str = "reviews";

use crate::model::Review;
use actix_web::{delete, get, post, web, HttpResponse};
use mongodb::{
    bson::{doc, Bson},
    Client, Collection,
};
use serde::Deserialize;
use serde_json::json;
use uuid::Uuid;

#[delete["/reviews/{reviewID}/comments/{commentID}"]]
pub async fn delete_comment(
    Auth(_user): Auth,
    client: web::Data<Client>,
    path: web::Path<(String, String)>,
) -> HttpResponse {
    let (review_id, comment_id) = path.into_inner();
    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);

    let filter = doc! { "reviewID": review_id, "comments.commentID": comment_id.clone() };
    let update_doc = doc! { "$pull": { "comments": { "commentID": comment_id.clone() } } };

    match collection.update_one(filter, update_doc, None).await {
        Ok(result) => {
            if result.matched_count > 0 {
                HttpResponse::Ok().finish()
            } else {
                HttpResponse::NotFound().finish()
            }
        }
        Err(err) => {
            println!("DB error: {:?}", err);
            HttpResponse::InternalServerError().finish()
        }
    }
}

#[derive(Deserialize)]
pub struct PostComment {
    pub comment: String,
}
#[post("/reviews/{id}/comments")]
pub async fn post_comment(
    Auth(_): Auth,
    req: web::Json<PostComment>,
    id: web::Path<String>,
    client: web::Data<Client>,
) -> HttpResponse {
    println!("function called");
    let id = id.into_inner();
    if req.comment.is_empty() {
        return HttpResponse::BadRequest().finish();
    }

    let collection = client.database(DB_NAME).collection::<Review>(COLL_NAME);

    let update_doc = doc! { "$push": doc! {"comments": doc! { "comment": req.comment.clone(), "commentID": Bson::String(Uuid::new_v4().to_string()), "userID": "_"}}};

    match collection
        .update_one(doc! { "reviewID": id}, update_doc, None)
        .await
    {
        Ok(result) => {
            if result.matched_count > 0 {
                HttpResponse::Ok().finish()
            } else {
                HttpResponse::NotFound().json(json! ({ "error": "review not found"}))
            }
        }
        Err(_e) => HttpResponse::InternalServerError().finish(),
    }
}

#[get("/reviews/{id}/comments")]
pub async fn get_comments(id: web::Path<String>, client: web::Data<Client>) -> HttpResponse {
    let id = id.into_inner();

    let collection = client.database(DB_NAME).collection::<Review>(COLL_NAME);

    let result = match collection.find_one(doc! {"reviewID": id}, None).await {
        Ok(cursor) => cursor,
        Err(err) => {
            eprintln!("Error creating cursor: {:?}", err);
            return HttpResponse::InternalServerError().body("Error fetching reviews");
        }
    };

    let comments = result.unwrap().comments;

    HttpResponse::Ok().json(comments)
}
