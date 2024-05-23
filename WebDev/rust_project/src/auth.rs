// credit to this amazing man for the auth code: https://www.youtube.com/watch?v=n2M4A4mO0QU&ab_channel=cudidotdev
// https://github.com/cudidotdev/JWT-Authentication-with-Rust-Axum-and-Actix/blob/main/actix-auth/src/main.rs

const DB_NAME: &str = "WebDev";

use crate::jwt;
use actix_web::web::Json;
use actix_web::{get, post, web, HttpResponse};
use argon2::{
    password_hash::{rand_core::OsRng, PasswordHash, PasswordHasher, PasswordVerifier, SaltString},
    Argon2,
};
use mongodb::{bson::doc, Client};
use serde::Deserialize;
use serde::Serialize;
use serde_json::json;

#[derive(Deserialize, Serialize)]
pub struct Account {
    pub username: String,
    pub password: String,
}

#[derive(Deserialize, Serialize)]
struct DBAccount {
    pub username: String,
    pub password: String,
}

#[post("/signup")]
pub async fn signup(req: web::Json<Account>, client: web::Data<Client>) -> HttpResponse {
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
                    let user = DBAccount {
                        username: req.username.clone(),
                        password: hash_result.to_string(),
                    };

                    match collection.insert_one(user, None).await {
                        Ok(_) => HttpResponse::Created().json(json!({ "status": "logged in" })),
                        Err(_) => {
                            println!("failed to insert");
                            HttpResponse::InternalServerError().finish()
                        }
                    }
                }
                Err(_) => {
                    println!("failed to hash");
                    HttpResponse::InternalServerError().finish()
                }
            }
        }
        Err(error) => {
            println!("{:?}", error);
            HttpResponse::InternalServerError().finish()
        }
    }
}

#[post("/login")]
pub async fn login(req: web::Json<Account>, client: web::Data<Client>) -> HttpResponse {
    if req.username.is_empty() || req.password.is_empty() {
        return HttpResponse::BadRequest().finish();
    }

    let collection = client.database(DB_NAME).collection::<DBAccount>("accounts");

    let filter = doc! { "username": &req.username };
    match collection.find_one(filter, None).await {
        Ok(Some(db_acc)) => {
            let hashed_password = db_acc.password;
            println! {"hashed password: {:?}", hashed_password};

            let parsed_hash = match PasswordHash::new(&hashed_password) {
                Ok(hash) => hash,
                Err(_) => return HttpResponse::InternalServerError().finish(),
            };
            println! {"parsed password: {:?}", parsed_hash};

            if Argon2::default()
                .verify_password(req.password.as_bytes(), &parsed_hash)
                .is_ok()
            {
                let user = jwt::User {
                    username: req.username.clone(),
                    password: req.password.clone(),
                };
                let token = jwt::get_jwt(user);

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
                println! {"password verification failed"};
                HttpResponse::InternalServerError().finish()
            }
        }
        Ok(None) => HttpResponse::NotFound().finish(),
        Err(_) => {
            println!("db lookup failed\n");
            HttpResponse::InternalServerError().finish()
        }
    }
}

#[post("/logout")]
pub async fn logout() -> HttpResponse {
    let mut no_more_cookies =
        actix_web::cookie::Cookie::build("jwt_token", "no more milk either").finish();
    no_more_cookies.make_removal();

    HttpResponse::Ok().cookie(no_more_cookies).finish()
}

pub async fn get_token_handler(Json(user): Json<jwt::User>) -> HttpResponse {
    let token = jwt::get_jwt(user);

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

pub async fn secret_view_handler(jwt::Auth(user): jwt::Auth) -> HttpResponse {
    HttpResponse::Ok().json(json!({
      "success": true,
      "data": user
    }))
}
