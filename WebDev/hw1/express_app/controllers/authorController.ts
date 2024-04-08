import { Request, Response } from "express";
import { authors, Author } from "../data";

export const add_author = (req: Request, res: Response): void => {
  const { name, bio, bday, genre } = req.body;
  if (!name || !bio || !bday || !genre) {
    res
      .status(400)
      .json({ error: "Please provide name, bio, bday, and genre" });
    return;
  }
  const author = new Author(name, bio, bday, genre);
  console.log(req.body);
  authors.push(author);
  res.status(200).json({ author });
};

export const list_authors = (req: Request, res: Response): void => {
  res.status(200).json({ authors });
};

export const update_author_bio = (req: Request, res: Response): void => {
  const authorID = req.params.authorID;
  if (!authorID) {
    res.status(400).json({ error: "Please provide an authorID" });
    return;
  }
  const { bio } = req.body;
  const author = authors.find((author) => author.id === authorID);
  if (!author) {
    res.status(404).json({ message: "Author not found" });
    return;
  }
  author.bio = bio;
  res.status(200).json({ author });
};
