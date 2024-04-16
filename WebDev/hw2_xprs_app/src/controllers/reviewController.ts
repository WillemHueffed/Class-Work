import { Request, Response } from "express";
import { mongoDB } from "../index";

export const create_review = async (
  req: Request,
  res: Response,
): Promise<void> => {
  try {
    const reviewCollection = mongoDB.collection("reviews");
    const rev = { "1": "test" };
    const result = await reviewCollection.insertOne(rev);
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
