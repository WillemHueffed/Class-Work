mod model;
//use actix_web::{get, post, web, App, HttpResponse, HttpServer, Responder};
//use mongodb::{bson::doc, options::IndexOptions, Client, Collection, IndexModel};
use actix_web::{delete, get, patch, post, web, App, HttpResponse, HttpServer};
use futures::stream::StreamExt;
use model::{Book, Comment, PostReview, Review};
use mongodb::{
    bson::{doc, document, oid::ObjectId, Bson},
    Client, Collection,
};
use serde::Deserialize;
use serde_json::json;
use uuid::Uuid;

const DB_NAME: &str = "WebDev";
const COLL_NAME: &str = "reviews";

#[derive(Deserialize)]
struct PatchReview {
    rating: String,
    desc: String,
}

// TODO: Add auth check
#[patch("/reviews/{id}")]
async fn patch_review(
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
                return HttpResponse::Ok().finish();
            } else {
                return HttpResponse::NotFound().finish();
            }
        }
        Err(err) => {
            println!("DB error: {:?}", err);
            return HttpResponse::InternalServerError().finish();
        }
    }
}

// TODO: Add auth check
#[delete["/reviews/{reviewID}/comments/{commentID}"]]
async fn delete_comment(
    client: web::Data<Client>,
    path: web::Path<(String, String)>,
) -> HttpResponse {
    let (review_id, comment_id) = path.into_inner();
    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);

    let filter = doc! { "reviewID": review_id, "comments.commentID": comment_id.clone() };
    //let update_doc = doc! { "$pull": doc! { "comments.commentID": comment_id.clone() }};
    let update_doc = doc! { "$pull": { "comments": { "commentID": comment_id.clone() } } };

    match collection.update_one(filter, update_doc, None).await {
        Ok(result) => {
            if result.matched_count > 0 {
                return HttpResponse::Ok().finish();
            } else {
                return HttpResponse::NotFound().finish();
            }
        }
        Err(err) => {
            println!("DB error: {:?}", err);
            return HttpResponse::InternalServerError().finish();
        }
    }
}

#[get("/reviews/byAuthor/{id}")]
async fn get_reviews_by_author(client: web::Data<Client>, id: web::Path<String>) -> HttpResponse {
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
async fn get_reviews_by_book(client: web::Data<Client>, id: web::Path<String>) -> HttpResponse {
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

#[derive(Deserialize)]
struct PostComment {
    comment: String,
}
//#[post("/reviews/{id}/comments")]
#[post("/reviews/{id}/comments")]
async fn post_comment(
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
                return HttpResponse::Ok().finish();
            } else {
                return HttpResponse::NotFound().json(json! ({ "error": "review not found"}));
            }
        }
        Err(_e) => HttpResponse::InternalServerError().finish(),
    }
}

#[get("/reviews/{id}/comments")]
async fn get_comments(id: web::Path<String>, client: web::Data<Client>) -> HttpResponse {
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

#[post("/reviews/{bookID}")]
async fn create_review(
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
        username: "wip".to_string(),
        userID: "wip".to_string(),
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

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // Parse a connection string into an options struct.
    let uri = "mongodb+srv://whueffed:whueffed@webdev.nque75t.mongodb.net/?retryWrites=true&w=majority&appName=WebDev";
    let client = Client::with_uri_str(uri)
        .await
        .expect("failed to connected");

    HttpServer::new(move || {
        App::new()
            .app_data(web::Data::new(client.clone()))
            .service(get_reviews_by_author)
            .service(get_reviews_by_book)
            .service(create_review)
            .service(post_comment)
            .service(get_comments)
            .service(patch_review)
            .service(delete_comment)
    })
    .bind(("localhost", 3002))?
    .run()
    .await
}
