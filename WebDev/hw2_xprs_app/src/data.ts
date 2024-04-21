import { v4 as uuidv4 } from "uuid";

class Review {
  rating: number;
  desc: string;
  comments: string[];
  reviewID: string;
  bookID: string;
  authorID: string;

  constructor(rating: number, desc: string, bookID: string, authorID: string) {
    this.reviewID = uuidv4();
    this.bookID = bookID;
    this.authorID = authorID;
    this.rating = rating;
    this.desc = desc;
    this.comments = [];
  }
}

export { Review };
