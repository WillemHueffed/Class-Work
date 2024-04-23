import { Request, Response } from "express";
import { mongoDB } from "../index";
import { ObjectId, PullOperator } from "mongodb";
import { Comment } from "../data";

export const create_comment = async (
  req: Request,
  res: Response,
): Promise<void> => {
  try {
    const reviewId = req.params.reviewID;
    const { comment } = req.body;
    console.log("in create comment");

    if (!comment) {
      res.status(400).json({ error: "Comment is required" });
      return;
    }

    const commentObj = new Comment(comment);
    const reviews = mongoDB.collection("reviews");
    const result = await reviews.updateOne(
      { reviewID: reviewId },
      { $push: { comments: commentObj } as PullOperator<Document> },
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

export const delete_comment = async (
  req: Request,
  res: Response,
): Promise<void> => {
  try {
    const reviewId = req.params.reviewID;
    const commentId = req.params.commentID;

    const reviews = mongoDB.collection("reviews");
    const result = await reviews.updateOne(
      {
        reviewID: reviewId,
        "comments.commentID": commentId,
      },
      {
        $pull: {
          comments: { commentID: commentId },
        } as PullOperator<Document>,
      },
    );

    if (result.matchedCount === 0) {
      res.status(404).json({ error: "Review or Comment not found" });
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
