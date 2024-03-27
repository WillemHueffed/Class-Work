import { Request, Response } from "express";
import { authors, Author } from "../data";

export const add_author = (req: Request, res: Response): void => {
  const { full_name, bio, bday, p_genre } = req.body;
  const author = new Author(full_name, bio, bday, p_genre);
  console.log(req.body);
  authors.push(author);
  res.status(201).json({ message: "Author created successfully", author });
};

export const list_authors = (req: Request, res: Response): void => {
  res.status(200).json(authors);
};

export const update_author_bio = (req: Request, res: Response): void => {
  const { author_name } = req.params;
  const { bio } = req.body;
  const author = authors.find((author) => author.name === author_name);
  if (!author) {
    res.status(404).json({ message: "Author not found" });
    return;
  }
  author.bio = bio;
  res.status(200).json({ message: "Author bio updated successfully", author });
};
