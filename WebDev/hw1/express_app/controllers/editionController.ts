import { Request, Response } from "express";
import { books, Edition } from "../data";

export const add_edition = (req: Request, res: Response): void => {
  const bookID = req.params.bookID as string;
  const book = books.find((book) => book.id === bookID);
  if (!book) {
    res.status(404).json({ error: "Book not found" });
    return;
  }
  const { ed_number, pub_date } = req.body;
  const edition = new Edition(ed_number, pub_date);
  book.editions.push(edition);

  res.status(200).json({ message: "Book edition created sucsessfully", book });
};

export const list_editions = (req: Request, res: Response): void => {
  const bookID = req.params.id as string;
  const book = books.find((book) => book.id === bookID);
  if (!book) {
    res.status(404).json({ error: "Book not found" });
    return;
  }
  const editions = book.editions;

  res.status(200).json({ editions });
};

export const delete_edition = (req: Request, res: Response): void => {
  const bookID = req.params.id as string;
  const editionID = req.params.editionID as string;
  const book = books.find((book) => book.id === bookID);
  if (!book) {
    res.status(404).json({ error: "Book not found" });
    return;
  }
  const editionIndex = book.editions.findIndex(
    (edition) => edition.id === editionID,
  );
  if (editionIndex === -1) {
    res.status(404).json({ error: "Edition not found" });
    return;
  }
  book.editions.splice(editionIndex, 1);

  res.status(200).json({ message: "Edition deleted successfully" });
};
