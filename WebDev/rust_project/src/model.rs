use serde::{Deserialize, Serialize};

#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]

pub struct Comment{
  pub comment: String,
  pub comment_id: String,
  pub user_id: String,
}

#[derive(Clone, Debug, PartialEq, Eq, Deserialize, Serialize)]

pub struct Review{
  rating: i32,
  desc: String,
  comments: Vec<Comment>,
  review_id: String,
  book_id: String,
  author_id: String,
  user_id: String,
  username: String,
}
