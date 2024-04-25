import { Request, Response } from "express";
import { mongoDB } from "../index";
import { Review } from "../data";

interface Author {
  name: string;
  bio: string;
  bday: string;
  genre: string;
  id: string;
}

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
  let books;

  try {
    const response = await fetch("http://localhost:3000/books");
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
  console.log(books);
  const specifiedBook = books.find((book: Book) => book.id === bookID);

  if (!specifiedBook) {
    res.status(404).json({ error: "Book not found\n" });
    return;
  }

  const reviewCollection = mongoDB.collection("reviews");

  const userID = req.oidc.user?.sub;
  const existingReview = await reviewCollection.findOne({
    bookID: bookID,
    userID: userID,
  });

  if (existingReview) {
    res.status(400).json({ error: "User has already reviewed this book" });
    return;
  }

  const username = req.oidc.user?.nickname;
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
    specifiedBook.id,
    userID,
    username,
  );
  try {
    const result = await reviewCollection.insertOne(review);
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

export const patch_review_by_id = async (
  req: Request,
  res: Response,
): Promise<void> => {
  console.log("in patch");
  const reviewID = req.params.reviewID;
  console.log(req.body);
  const updateData: { rating?: string; desc?: string } = {};

  const { rating, description } = req.body;
  if (rating) {
    updateData.rating = rating;
  }
  if (description) {
    updateData.desc = description;
  }

  if (!rating && !description) {
    res.status(400).json({ error: "Please update rating and/or description" });
    return;
  }

  try {
    const reviews = mongoDB.collection("reviews");
    const userID = req.oidc.user?.sub;
    const result = await reviews.updateOne(
      { reviewID: reviewID, userID: userID },
      { $set: updateData },
    );

    if (result.matchedCount === 0) {
      console.log("no review found");
      res.status(404).json({ error: "Review not found" });
      return;
    }
    console.log("200");
    res.status(200).json({ message: "Review updated successfully" });
  } catch (error) {
    console.error("Error updating review:", error);
    res.status(500).json({ error: "Internal server error" });
  }
};

export const get_reviews_by_bookID = async (
  req: Request,
  res: Response,
): Promise<void> => {
  let books;
  try {
    const response = await fetch("http://localhost:3000/books");
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

  if (!specifiedBook) {
    res.status(404).json({ error: "Book not found\n" });
    return;
  }

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
  let authors;
  try {
    const response = await fetch("http://localhost:3000/authors");
    if (!response.ok) {
      console.error("Bad resonse from localhost:3000/books");
      res.status(500).json({ error: "Internal server error" });
      return;
    }
    authors = await response.json();
  } catch (error) {
    console.error("Error fetching data:", error);
    res.status(500).json({ error: "Internal server error" });
    return;
  }

  console.log(authors);
  const specifiedAuthor = authors.find(
    (author: Author) => author.id === req.params.authorID,
  );

  // Log the specified book if found
  if (!specifiedAuthor) {
    res.status(404).json({ error: "Author not found\n" });
    return;
  }

  const reviews = mongoDB.collection("reviews");
  const objArr = await reviews.find({ bookID: req.params.authorID }).toArray();
  const mappedArr = objArr.map(({ _id, comments, ...rest }) => rest);
  const stringArr = JSON.stringify(mappedArr);
  const jsonArr = JSON.parse(stringArr);
  res.status(200).json(jsonArr);
};
