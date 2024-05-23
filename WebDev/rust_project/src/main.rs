mod auth;
mod comment;
mod html;
mod jwt;
mod model;
mod review;
use actix_web::{web, web::get, web::post, App, HttpServer};
use mongodb::Client;

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
            .route(
                "/delete_comment.html",
                web::get().to(html::html_delete_comment),
            )
            .route("/get_authors.html", web::get().to(html::html_get_authors))
            .route(
                "/get_comments_by_review.html",
                web::get().to(html::html_get_comments_by_review),
            )
            .route(
                "/get_reviews_by_author.html",
                web::get().to(html::html_get_reviews_by_author),
            )
            .route(
                "/get_reviews_by_book.html",
                web::get().to(html::html_get_reviews_by_book),
            )
            .route("/signup", web::get().to(html::html_signup))
            .route("/login", web::get().to(html::html_login))
            .route("/logout", web::get().to(html::html_logout))
            .route("/get-token", post().to(auth::get_token_handler))
            .route("/secret-view", get().to(auth::secret_view_handler))
            .route("/patch_review.html", web::get().to(html::html_patch_review))
            .route("/post_comment.html", web::get().to(html::html_post_comment))
            .route("/post_review.html", web::get().to(html::html_post_review))
            .service(review::get_reviews_by_author)
            .service(review::get_reviews_by_book)
            .route("/reviews/{id}", web::post().to(review::create_review))
            .service(comment::post_comment)
            .service(comment::get_comments)
            .service(review::patch_review)
            .service(comment::delete_comment)
            .service(auth::signup)
            .service(auth::login)
            .service(auth::logout)
            .app_data(web::Data::new(client.clone()))
    })
    .bind(("localhost", 3002))?
    .run()
    .await
}
