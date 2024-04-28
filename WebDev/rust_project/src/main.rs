mod model;
use actix_web::{get, post, web, App, HttpResponse, HttpServer, Responder};
//use mongodb::{
//   bson::doc, options::ClientOptions, options::IndexOptions, Client, Collection, IndexModel,
//};
use mongodb::{bson::doc, options::IndexOptions, Client, Collection, IndexModel};
use model::{Comment, Review};

const DB_NAME: &str = "WebDev";
const COLL_NAME: &str = "reviews";

#[get("/reviews")]
async fn get_reviews(client: web::Data<client>, username: web::Path<string>) -> HttpResponse{
    let collection: Collection<Review> = client.database(DB_NAME).collection(COLL_NAME);
    // Retrieve documents from the collection
    match collection.find(None, None).await {
        Ok(cursor) => {
            // Iterate over the documents and print them to the terminal
            while let Some(doc) = cursor.next().await {
                match doc {
                    Ok(document) => println!("{:?}", document),
                    Err(e) => eprintln!("Error reading document: {}", e),
                }
            }
        }
        Err(e) => eprintln!("Error retrieving documents from collection: {}", e),
    }
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
    })
    .bind(("localhost", 3002))?
    .run()
    .await
}
