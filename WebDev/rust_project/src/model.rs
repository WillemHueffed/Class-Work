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

#[derive(Deserialize)]
pub struct PostReview {
    pub description: String,
    pub rating: String,
}
