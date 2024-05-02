mod model;

//use actix_web::{get, post, web, App, HttpResponse, HttpServer, Responder};
//use mongodb::{bson::doc, options::IndexOptions, Client, Collection, IndexModel};
use model::{Comment, Review};
use actix_web::{get, web, App, HttpResponse, HttpServer, Responder};
use mongodb::{bson::doc, options::FindOptions, Client, Collection, error::Error};
use futures::stream::StreamExt;

const DB_NAME: &str = "WebDev";
const COLL_NAME: &str = "reviews";

#[get("/reviews/byAuthor/{id}")]
async fn get_reviews(client: web::Data<Client>, id: web::Path<String>) -> HttpResponse{
    let id = id.into_inner();
    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);
    match collection
        .find_one(doc! { "review_id": &id}, None)
        .await
    {
        Ok(Some(user)) => HttpResponse::Ok().json(user),
        Ok(None) => {
            HttpResponse::NotFound().body(format!("No user found with username {id}"))
        }
        Err(err) => HttpResponse::InternalServerError().body(err.to_string()),
    }
}

#[get("/")]
async fn hello_world() -> impl Responder{
    HttpResponse::Ok().body("hello world\n")
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // Parse a connection string into an options struct.
    let uri = std::env::var("MONGODB_URI").unwrap_or_else(|_| "mongodb://localhost:27017".into());
    let client = Client::with_uri_str(uri).await.expect("failed to connected");

    HttpServer::new(move || {
        App::new()
            .app_data(web::Data::new(client.clone()))
            .service(get_reviews)
            .service(hello_world)
    })
    .bind(("localhost", 3002))?
    .run()
    .await
}
