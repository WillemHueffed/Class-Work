mod model;

//use actix_web::{get, post, web, App, HttpResponse, HttpServer, Responder};
//use mongodb::{bson::doc, options::IndexOptions, Client, Collection, IndexModel};
use actix_web::{get, web, App, HttpResponse, HttpServer, Responder};
use futures::stream::{StreamExt, TryStreamExt};
use model::{Comment, Review};
use mongodb::{bson::doc, error::Error, options::FindOptions, Client, Collection};

const DB_NAME: &str = "WebDev";
const COLL_NAME: &str = "reviews";

#[get("/reviews/byAuthor/{id}")]
async fn get_reviews(client: web::Data<Client>, id: web::Path<String>) -> HttpResponse {
    let id = id.into_inner();
    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);
    match collection.find_one(doc! { "review_id": &id}, None).await {
        Ok(Some(user)) => HttpResponse::Ok().json(user),
        Ok(None) => HttpResponse::NotFound().body(format!("No user found with username {id}")),
        Err(err) => HttpResponse::InternalServerError().body(err.to_string()),
    }
}

#[get("/")]
async fn hello_world(client: web::Data<Client>) -> HttpResponse {
    let collection = client.database(DB_NAME).collection::<Review>(COLL_NAME);
    let mut cursor = collection.find(doc! {}, None).await.expect("REASON");
    while cursor.advance().await.expect("reason") {
        println!("{:?}", cursor.deserialize_current());
    }
    HttpResponse::Ok().into()
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
            .service(get_reviews)
            .service(hello_world)
    })
    .bind(("localhost", 3002))?
    .run()
    .await
}
