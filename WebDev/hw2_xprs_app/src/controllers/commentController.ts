import { Request, Response } from "express";
import { mongoDB } from "../index";
import { PullOperator } from "mongodb";
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

    const userID = req.oidc.user?.sub;

    const commentObj = new Comment(comment, userID);
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
    const userID = req.oidc.user?.sub;

    const reviews = mongoDB.collection("reviews");
    //'console.log("commentid");
    //console.log(commentId);
    //console.log("userID");
    //console.log(userID);
    const result = await reviews.updateOne(
      {
        reviewID: reviewId,
        "comments.commentID": commentId,
        "comments.userID": userID,
      },
      {
        $pull: {
          comments: { commentID: commentId, userID: userID },
        } as PullOperator<Document>,
      },
    );

    //console.log(result);

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

export const get_comments = async (
  req: Request,
  res: Response,
): Promise<void> => {
  // TODO: the thign
  console.log(req.oidc.user);
  const reviews = mongoDB.collection("reviews");
  try {
    const review = await reviews.findOne({ reviewID: req.params.reviewID });
    if (!review) {
      res.status(404).json({ error: "Review not found" });
      return;
    }
    res.status(200).json(review.comments);
  } catch (error) {
    console.error("Error retrieving comments:", error);
    res.status(500).json({ error: "Internal server error" });
  }
};
