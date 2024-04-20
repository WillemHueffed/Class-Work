import { Request, Response } from "express";
import { mongoDB } from "../index";
import { Review } from "../data";

export const create_review = async (
  req: Request,
  res: Response,
): Promise<void> => {
  console.log("in create_review");
  const { rating, description } = req.body;
  if (!rating || !description) {
    res.status(400).json({ error: "Please provide a rating and a comment" });
    return;
  }
  const review = new Review(rating, description);
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

export const get_reviews_by_bookID = (req: Request, res: Response): void => {
  res.status(404).json({ error: "wip list reviews by book id" });
};

export const get_reviews_by_authorID = (req: Request, res: Response): void => {
  res.status(404).json({ error: "wip list reviews by author id" });
};
