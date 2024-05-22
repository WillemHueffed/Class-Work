#![allow(non_snake_case)]

use serde::{Deserialize, Serialize};

#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]

pub struct Comment {
    pub comment: String,
    pub commentID: String,
    pub userID: String,
}

#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]

pub struct Review {
    pub rating: String,
    pub desc: String,
    pub comments: Vec<Comment>,
    pub reviewID: String,
    pub bookID: String,
    pub authorID: String,
    pub userID: String,
    pub username: String,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Edition {
    pub ed_num: i32,
    pub pub_date: String,
    pub id: String,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Author {
    pub name: String,
    pub bio: String,
    pub bday: String,
    pub genre: String,
    pub id: String,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Book {
    pub title: String,
    pub subtitle: String,
    pub org_pub_date: String,
    pub tags: Vec<String>,
    pub p_auth: Author,
    pub id: String,
    pub editions: Vec<Edition>,
}

#[derive(Debug, Deserialize)]
pub struct PostReview {
    pub desc: String,
    pub rating: String,
}
