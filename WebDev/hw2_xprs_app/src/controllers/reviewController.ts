import { Request, Response } from "express";
import { mongoDB } from "../index";
import { Review } from "../data";

interface Book {
  title: string;
  subtitle: string;
  org_pub_date: string;
  tags: string[];
  p_auth: {
    name: string;
    bio: string;
    bday: string;
    genre: string;
    id: string;
  };
  id: string;
  editions: { ed_num: number; pub_date: string; id: string }[];
}

export const create_review = async (
  req: Request,
  res: Response,
): Promise<void> => {
  console.log("wtf\n");
  let books;

  try {
    console.log("trying to ping server");
    const response = await fetch("http://localhost:3000/books");
    console.log("pinged");
    if (!response.ok) {
      console.error("Bad resonse from localhost:3000/books");
      res.status(500).json({ error: "Internal server error" });
      return;
    }
    books = await response.json();
  } catch (error) {
    console.error("Error fetching data:", error);
    res.status(500).json({ error: "Internal server error" });
    return;
  }

  const bookID = req.params.bookID;
  const specifiedBook = books.find((book: Book) => book.id === bookID);

  // Log the specified book if found
  if (!specifiedBook) {
    res.status(404).json({ error: "Book not found\n" });
    return;
  }

  const { rating, description } = req.body;
  if (!rating || !description) {
    res
      .status(400)
      .json({ error: "Please provide a rating and a description" });
    return;
  }
  const review = new Review(
    rating,
    description,
    bookID,
    specifiedBook.authorID,
  );
  try {
    const reviewCollection = mongoDB.collection("reviews");
    const result = await reviewCollection.insertOne(review);
    console.log(`A document was inserted with the _id: ${result.insertedId}`);
    res.status(201).json({
      message: "Review created successfully",
      insertedId: result.insertedId,
    });
  } catch (error) {
    console.error("Error inserting review:", error);
    console.log(error);
    res.status(500).json({ message: "Error creating review" });
  }
};

export const patch_review_by_id = (req: Request, res: Response): void => {
  res.status(404).json({ error: "wip patch review" });
};

export const get_reviews_by_bookID = async (
  req: Request,
  res: Response,
): Promise<void> => {
  const reviews = mongoDB.collection("reviews");
  const objArr = await reviews.find({ bookID: req.params.bookID }).toArray();
  const mappedArr = objArr.map(({ _id, comments, ...rest }) => rest);
  const stringArr = JSON.stringify(mappedArr);
  const jsonArr = JSON.parse(stringArr);
  res.status(200).json(jsonArr);
};

export const get_reviews_by_authorID = async (
  req: Request,
  res: Response,
): Promise<void> => {
  const reviews = mongoDB.collection("reviews");
  const objArr = await reviews.find({ bookID: req.params.authorID }).toArray();
  const mappedArr = objArr.map(({ _id, comments, ...rest }) => rest);
  const stringArr = JSON.stringify(mappedArr);
  const jsonArr = JSON.parse(stringArr);
  res.status(200).json(jsonArr);
};
