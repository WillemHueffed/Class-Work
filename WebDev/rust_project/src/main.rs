// credit to this amazing man for the auth code: https://www.youtube.com/watch?v=n2M4A4mO0QU&ab_channel=cudidotdev
// https://github.com/cudidotdev/JWT-Authentication-with-Rust-Axum-and-Actix/blob/main/actix-auth/src/main.rs

mod model;
use actix_web::{
    delete, error::InternalError, get, http::StatusCode, patch, post, web, web::get, web::post,
    App, HttpResponse, HttpServer,
};
use actix_web::{dev::Payload, http::header, web::Json, FromRequest, HttpRequest};
use argon2::{
    password_hash::{rand_core::OsRng, PasswordHash, PasswordHasher, PasswordVerifier, SaltString},
    Argon2,
};
use chrono::{Duration, Utc};
use futures::stream::StreamExt;
use jsonwebtoken::{decode, encode, DecodingKey, EncodingKey, Header, Validation};
use model::{Book, PostReview, Review};
use mongodb::{
    bson::{doc, Bson},
    Client, Collection,
};
use serde::Deserialize;
use serde::Serialize;
use serde_json::json;
use std::future::{ready, Ready};
use uuid::Uuid;
const DB_NAME: &str = "WebDev";
const COLL_NAME: &str = "reviews";

#[derive(Deserialize, Serialize)]
pub struct User {
    username: String,
    password: String,
}

#[derive(Serialize, Deserialize)]
struct Claims {
    username: String,
    exp: i64,
}

pub fn get_jwt(user: User) -> Result<String, String> {
    let token = encode(
        &Header::default(),
        &Claims {
            username: user.username,
            exp: (Utc::now() + Duration::minutes(1000)).timestamp(),
        },
        &EncodingKey::from_secret("mykey".as_bytes()),
    )
    .map_err(|e| e.to_string());

    return token;
}

pub fn decode_jwt(token: &str) -> Result<User, String> {
    let token_data = decode::<User>(
        token,
        &DecodingKey::from_secret("mykey".as_bytes()),
        &Validation::default(),
    );

    match token_data {
        Ok(token_data) => Ok(token_data.claims),

        Err(e) => Err(e.to_string()),
    }
}

struct Auth(User);

impl FromRequest for Auth {
    type Error = InternalError<String>;

    type Future = Ready<Result<Self, Self::Error>>;

    fn from_request(req: &HttpRequest, _: &mut Payload) -> Self::Future {
        let access_token = req
            .headers()
            .get(header::AUTHORIZATION)
            .and_then(|value| value.to_str().ok())
            .and_then(|str| str.split(" ").nth(1));

        match access_token {
            Some(token) => {
                let user = decode_jwt(token);

                match user {
                    Ok(user) => ready(Ok(Auth(user))),

                    Err(e) => ready(Err(InternalError::from_response(
                        e.clone(),
                        HttpResponse::Unauthorized().json(json!({
                          "success": false,
                          "data": {
                            "message": e
                          }
                        })),
                    ))),
                }
            }

            None => ready(Err(InternalError::from_response(
                String::from("No token provided"),
                HttpResponse::Unauthorized().json(json!({
                  "success": false,
                  "data": {
                    "message": "No token provided"
                  }
                })),
            ))),
        }
    }
}

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

#[derive(Deserialize, Serialize)]
struct Account {
    username: String,
    password: String,
}

#[derive(Deserialize, Serialize)]
struct DBAccount {
    username: String,
    salt: String,
    password: String,
}

/*
#[post("/signup")]
async fn signup(req: web::Json<Account>, client: web::Data<Client>) -> HttpResponse {
    if req.username.is_empty() || req.password.is_empty() {
        return HttpResponse::BadRequest().finish();
    }

    let collection = client.database(DB_NAME).collection::<DBAccount>("accounts");

    let filter = doc! { "username": &req.username };
    match collection.find_one(filter, None).await {
        Ok(Some(_)) => HttpResponse::Conflict().finish(),
        Ok(None) => {
            let password = req.password.clone();
            match bcrypt::hash_with_result(&password, 4) {
                Ok(hash) => {
                    let user = DBAccount {
                        username: req.username.clone(),
                        password: hash.to_string(),
                        salt: hash.get_salt(),
                    };
                    match collection.insert_one(user, None).await {
                        Ok(_) => HttpResponse::Created().finish(),
                        Err(_) => HttpResponse::InternalServerError().finish(),
                    }
                }
                Err(_) => HttpResponse::InternalServerError().finish(),
            }
        }
        Err(_) => HttpResponse::InternalServerError().finish(),
    }
}
*/

#[post("/signup")]
async fn signup(req: web::Json<Account>, client: web::Data<Client>) -> HttpResponse {
    if req.username.is_empty() || req.password.is_empty() {
        return HttpResponse::BadRequest().finish();
    }

    let collection = client.database(DB_NAME).collection::<DBAccount>("accounts");

    let filter = doc! { "username": &req.username };
    match collection.find_one(filter, None).await {
        Ok(Some(_)) => HttpResponse::Conflict().finish(),
        Ok(None) => {
            let password = req.password.clone();
            let salt = SaltString::generate(&mut OsRng);
            // Argon2 with default params (Argon2id v19)
            let argon2 = Argon2::default();

            // Hash password to PHC string ($argon2id$v=19$...)
            match argon2.hash_password(password.as_bytes(), &salt) {
                Ok(hash_result) => {
                    let hashed_password = hash_result.hash.unwrap();
                    let user = DBAccount {
                        username: req.username.clone(),
                        password: hashed_password.to_string(),
                        salt: salt.to_string(),
                    };

                    match collection.insert_one(user, None).await {
                        Ok(_) => HttpResponse::Created().finish(),
                        Err(_) => HttpResponse::InternalServerError().finish(),
                    }
                }
                Err(_) => HttpResponse::InternalServerError().finish(),
            }
        }
        Err(_) => HttpResponse::InternalServerError().finish(),
    }
}

#[post("/login")]
async fn login(req: web::Json<Account>, client: web::Data<Client>) -> HttpResponse {
    if req.username.is_empty() || req.password.is_empty() {
        return HttpResponse::BadRequest().finish();
    }

    let collection = client.database(DB_NAME).collection::<DBAccount>("accounts");

    let filter = doc! { "username": &req.username };
    match collection.find_one(filter, None).await {
        Ok(Some(db_acc)) => {
            let hashed_password = db_acc.password;
            let salt_slice = db_acc.salt.as_bytes();

            let salt: [u8; 16] = salt_slice.try_into().expect("slice with incorrect length");

            // Borrow the password field instead of moving it
            let password = &req.password;
            let check = bcrypt::hash_with_salt(password, 4, salt);
            let check = check.unwrap();
            if hashed_password == check.to_string() {
                println!("verified");
                let user = User {
                    username: req.username.clone(),
                    password: req.password.clone(),
                };
                let token = get_jwt(user);

                match token {
                    Ok(token) => HttpResponse::Ok()
                        .cookie(
                            actix_web::cookie::Cookie::build("jwt_token", token.clone()).finish(),
                        )
                        .json(json!({
                          "success": true,
                          "data": {
                            "token": token
                          }
                        })),

                    Err(error) => HttpResponse::BadRequest().json(json!({
                      "success": false,
                      "data": {
                        "message": error
                      }
                    })),
                }
            } else {
                println!("not verified");
                HttpResponse::Unauthorized().finish()
            }
        }
        Ok(None) => HttpResponse::NotFound().finish(),
        Err(_) => HttpResponse::InternalServerError().finish(),
    }
}

/*&
#[post("/login")]
async fn login(req: web::Json<Account>, client: web::Data<Client>) -> HttpResponse {
    if req.username.is_empty() || req.password.is_empty() {
        return HttpResponse::BadRequest().finish();
    }

    let collection = client.database(DB_NAME).collection::<DBAccount>("accounts");

    let filter = doc! { "username": &req.username };
    match collection.find_one(filter, None).await {
        Ok(Some(db_acc)) => {
            let hashed_password = db_acc.password;
            let salt: [u8; 16] = db_acc.salt.as_bytes().try_into().unwrap();
            let password = &req.password;
            let mut check = bcrypt::hash_with_salt(password, 4, salt);
            let check = check.unwrap();
            if hashed_password == check.to_string() {
                println!("verified");
                let user = User {
                    username: req.username.clone(),
                    password: req.password.clone(),
                };
                let token = get_jwt(user);

                match token {
                    Ok(token) => HttpResponse::Ok()
                        .cookie(
                            actix_web::cookie::Cookie::build("jwt_token", token.clone()).finish(),
                        )
                        .json(json!({
                          "success": true,
                          "data": {
                            "token": token
                          }
                        })),

                    Err(error) => HttpResponse::BadRequest().json(json!({
                      "success": false,
                      "data": {
                        "message": error
                      }
                    })),
                }
            } else {
                println!("not verified");
                HttpResponse::Unauthorized().finish()
            }
        }
        Ok(None) => HttpResponse::NotFound().finish(),
        Err(_) => HttpResponse::InternalServerError().finish(),
    }
}
*/

#[get("/logout")]
async fn logout() -> HttpResponse {
    let mut no_more_cookies =
        actix_web::cookie::Cookie::build("jwt_token", "no more milk either").finish();
    no_more_cookies.make_removal();

    HttpResponse::Ok().cookie(no_more_cookies).finish()
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

async fn create_review(
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

async fn html_delete_comment() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/delete_comment.html"))
}

async fn html_get_authors() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/get_authors.html"))
}

async fn html_get_comments_by_review() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/get_comments_by_review.html"))
}

async fn html_get_reviews_by_author() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/get_reviews_by_author.html"))
}

async fn html_get_reviews_by_book() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/get_reviews_by_book.html"))
}

async fn html_patch_review() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/patch_review.html"))
}

async fn html_post_comment() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/post_comment.html"))
}

async fn html_post_review() -> HttpResponse {
    HttpResponse::build(StatusCode::OK)
        .content_type("text/html; charset=utf-8")
        .body(include_str!("static/post_review.html"))
}

async fn get_token_handler(Json(user): Json<User>) -> HttpResponse {
    let token = get_jwt(user);

    match token {
        Ok(token) => HttpResponse::Ok()
            .cookie(actix_web::cookie::Cookie::build("jwt_token", token.clone()).finish())
            .json(json!({
              "success": true,
              "data": {
                "token": token
              }
            })),

        Err(error) => HttpResponse::BadRequest().json(json!({
          "success": false,
          "data": {
            "message": error
          }
        })),
    }
}

async fn secret_view_handler(Auth(user): Auth) -> HttpResponse {
    HttpResponse::Ok().json(json!({
      "success": true,
      "data": user
    }))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // Parse a connection string into an options struct.
    let uri = "mongodb+srv://whueffed:whueffed@webdev.nque75t.mongodb.net/?retryWrites=true&w=majority&appName=WebDev";
    let client = Client::with_uri_str(uri)
        .await
        .expect("failed to connected");

    // I tried really hard to just serve the static directory directly but to no luck. stuff just
    // keeped returning 404s so this is my tedious work around
    HttpServer::new(move || {
        App::new()
            .route("/delete_comment.html", web::get().to(html_delete_comment))
            .route("/get_authors.html", web::get().to(html_get_authors))
            .route(
                "/get_comments_by_review.html",
                web::get().to(html_get_comments_by_review),
            )
            .route(
                "/get_reviews_by_author.html",
                web::get().to(html_get_reviews_by_author),
            )
            .route(
                "/get_reviews_by_book.html",
                web::get().to(html_get_reviews_by_book),
            )
            .route("/get-token", post().to(get_token_handler))
            .route("/secret-view", get().to(secret_view_handler))
            .route("/patch_review.html", web::get().to(html_patch_review))
            .route("/post_comment.html", web::get().to(html_post_comment))
            .route("/post_review.html", web::get().to(html_post_review))
            .service(get_reviews_by_author)
            .service(get_reviews_by_book)
            .route("/reviews/{id}", web::post().to(create_review))
            .service(post_comment)
            .service(get_comments)
            .service(patch_review)
            .service(delete_comment)
            .service(signup)
            .service(login)
            .service(logout)
            .app_data(web::Data::new(client.clone()))
    })
    .bind(("localhost", 3002))?
    .run()
    .await
}
