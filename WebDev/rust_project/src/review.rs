use crate::jwt::Auth;
use crate::model::Book;
use crate::model::PostReview;
use crate::model::Review;
use actix_web::{get, patch, web, HttpResponse};
use futures::StreamExt;
use mongodb::{bson::doc, Client, Collection};
use serde::Deserialize;
use serde_json::json;
use uuid::Uuid;

const DB_NAME: &str = "WebDev";
const COLL_NAME: &str = "reviews";

#[derive(Deserialize)]
pub struct PatchReview {
    pub rating: String,
    pub desc: String,
}

#[patch("/reviews/{id}")]
pub async fn patch_review(
    Auth(_user): Auth,
    client: web::Data<Client>,
    info: web::Json<PatchReview>,
    id: web::Path<String>,
) -> HttpResponse {
    let id = id.into_inner();

    if info.rating.is_empty() && info.desc.is_empty() {
        return HttpResponse::BadRequest().finish();
    }

    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);
    let filter = doc! { "reviewID": id };

    let update_doc;
    if info.rating.is_empty() && !info.desc.is_empty() {
        update_doc = doc! { "desc": info.desc.clone()};
    } else if !info.rating.is_empty() && info.desc.is_empty() {
        update_doc = doc! { "rating": info.rating.clone()};
    } else {
        update_doc = doc! { "desc": info.desc.clone(), "rating": info.rating.clone()};
    }
    match collection
        .update_one(filter, doc! { "$set": update_doc}, None)
        .await
    {
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

#[get("/reviews/byAuthor/{id}")]
pub async fn get_reviews_by_author(
    client: web::Data<Client>,
    id: web::Path<String>,
) -> HttpResponse {
    let id = id.into_inner();
    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);

    let filter = doc! { "authorID": id };

    let mut cursor = match collection.find(filter, None).await {
        Ok(cursor) => cursor,
        Err(err) => {
            eprintln!("Error creating cursor: {:?}", err);
            return HttpResponse::InternalServerError().body("Error fetching reviews");
        }
    };

    let mut reviews = Vec::new();
    while let Some(result) = cursor.next().await {
        match result {
            Ok(review) => reviews.push(review),
            Err(err) => {
                eprintln!("Error reading review: {:?}", err);
                return HttpResponse::InternalServerError().body("Error reading reviews");
            }
        }
    }

    HttpResponse::Ok().json(reviews)
}

#[get("/reviews/byBook/{id}")]
pub async fn get_reviews_by_book(client: web::Data<Client>, id: web::Path<String>) -> HttpResponse {
    let id = id.into_inner();
    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);

    let filter = doc! { "bookID": id };

    let mut cursor = match collection.find(filter, None).await {
        Ok(cursor) => cursor,
        Err(err) => {
            eprintln!("Error creating cursor: {:?}", err);
            return HttpResponse::InternalServerError().body("Error fetching reviews");
        }
    };

    let mut reviews = Vec::new();
    while let Some(result) = cursor.next().await {
        match result {
            Ok(review) => reviews.push(review),
            Err(err) => {
                eprintln!("Error reading review: {:?}", err);
                return HttpResponse::InternalServerError().body("Error reading reviews");
            }
        }
    }

    HttpResponse::Ok().json(reviews)
}

pub async fn create_review(
    Auth(user): Auth,
    path: web::Path<String>,
    info: web::Json<PostReview>,
    mongo_client: web::Data<Client>,
) -> HttpResponse {
    let book_id = path.into_inner();
    let desc = &info.desc;
    let rating = &info.rating;

    let res = reqwest::get("http://localhost:3000/books").await;
    let res_body = res.unwrap().text().await.unwrap();
    let books: Vec<Book> = serde_json::from_str(&res_body.to_string()).unwrap();

    #[allow(unused_assignments)]
    let mut author_id = "";
    if let Some(book) = books.iter().find(|book| book.id == book_id) {
        author_id = &book.p_auth.id;
    } else {
        return HttpResponse::NotFound().json(json!({ "error": "Book not found" }));
    }
    println!("Book with id {} found!", book_id);

    let review_id = Uuid::new_v4();

    let review = Review {
        rating: rating.to_string(),
        desc: desc.to_string(),
        bookID: book_id,
        reviewID: review_id.to_string(),
        comments: Vec::new(),
        username: user.username,
        authorID: author_id.to_string(),
    };

    let collection = mongo_client
        .database(DB_NAME)
        .collection::<Review>(COLL_NAME);

    let insert_res = collection.insert_one(review, None).await;

    if insert_res.is_err() {
        return HttpResponse::InternalServerError()
            .json(json!({ "error": "Internal server error - DB issue"}));
    }

    HttpResponse::Created().json(json!({ "reviewID": review_id}))
}
