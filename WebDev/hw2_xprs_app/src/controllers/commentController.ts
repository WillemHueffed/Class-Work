import { Request, Response } from "express";
import { mongoDB } from "../index";

// TODO: check that this is properly handling review and commment ids
export const addCommentToReview = async (
  req: Request,
  res: Response,
): Promise<void> => {
  try {
    const reviewId = req.params.reviewID;
    const { comment } = req.body;

    if (!comment) {
      res.status(400).json({ error: "Comment is required" });
      return;
    }

    const reviews = mongoDB.collection("reviews");
    const result = await reviews.updateOne(
      { reviewID: reviewId },
      { $push: { comments: comment } },
    );

    if (result.matchedCount === 0) {
      res.status(404).json({ error: "Review not found" });
      return;
    }

    res.status(200).json({ message: "Comment added successfully" });
  } catch (error) {
    console.error("Error adding comment to review:", error);
    res.status(500).json({ error: "Internal server error" });
  }
};

// TODO: this isn't handling ids properly
export const delete_comment = async (
  req: Request,
  res: Response,
): Promise<void> => {
  try {
    const reviewId = req.params.reviewID;
    const commentId = req.params.commentID;

    const reviews = mongoDB.collection("reviews");
    const result = await reviews.updateOne(
      { reviewID: reviewId },
      { $pull: { comments: { _id: new objectId(commentId) } } },
    );

    if (result.matchedCount === 0) {
      res.status(404).json({ error: "Review not found" });
      return;
    }

    res.status(200).json({ message: "Comment deleted successfully" });
  } catch (error) {
    console.error("Error deleting comment:", error);
    res.status(500).json({ error: "Internal server error" });
  }
};

// TODO: write this
export const get_comments = (req: Request, res: Response): void => {
  res.status(404).json({ error: "wip get comments" });
};
