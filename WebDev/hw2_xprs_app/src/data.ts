import { v4 as uuidv4 } from "uuid";

class Comment {
  comment: string;
  commentID: string;
  userID: string;

  constructor(comment: string, userID: string) {
    this.comment = comment;
    this.commentID = uuidv4();
    this.userID = userID;
  }
}

class Review {
  rating: number;
  desc: string;
  comments: Comment[];
  reviewID: string;
  bookID: string;
  authorID: string;
  userID: string;
  username: string;

  constructor(
    rating: number,
    desc: string,
    bookID: string,
    authorID: string,
    userID: string,
    username: string,
  ) {
    this.reviewID = uuidv4();
    this.bookID = bookID;
    this.authorID = authorID;
    this.rating = rating;
    this.desc = desc;
    this.comments = [];
    this.userID = userID;
    this.username = username;
  }
}

export { Review, Comment };
