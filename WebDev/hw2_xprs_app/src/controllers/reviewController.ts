import { Request, Response } from "express";

export const create_review = (req: Request, res: Response): void => {
  res.status(404).json({ error: "wip create review" });
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
