import { v4 as uuidv4 } from "uuid";

class Review {
  rating: number;
  desc: string;
  comments: string[];
  id: string;

  constructor(rating: number, desc: string) {
    this.id = uuidv4();
    this.rating = rating;
    this.desc = desc;
    this.comments = [];
  }
}

export { Review };
