import { Request, Response } from "express";

export const create_comment = (req: Request, res: Response): void => {
  const { comment } = req.body;
  console.log("registered the comment as");
  console.log(comment);
  res.status(404).json({ error: "wip create comment" });
};

export const delete_comment = (req: Request, res: Response): void => {
  res.status(404).json({ error: "wip delete comment" });
};

export const get_comments = (req: Request, res: Response): void => {
  res.status(404).json({ error: "wip get comments" });
};
