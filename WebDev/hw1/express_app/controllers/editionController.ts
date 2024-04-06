import { Request, Response } from "express";
import { books, Edition } from "../data";

export const add_edition = (req: Request, res: Response): void => {
  const bookID = req.params.bookID;
  const book = books.find((book) => book.id === bookID);
  if (!book) {
    res.status(404).json({ error: "Book not found" });
    return;
  }
  const { ed_num, pub_date } = req.body;
  if (!ed_num || !pub_date) {
    res.status(400).send({ error: "Please provide an ed_num and pub_date" });
    return;
  }
  const edition = new Edition(ed_num, pub_date);
  book.editions.push(edition);

  res.status(200).send(edition);
};

export const list_editions = (req: Request, res: Response): void => {
  const bookID = req.params.bookID;
  const book = books.find((book) => book.id === bookID);
  if (!book) {
    res.status(404).json({ error: "Book not found" });
    return;
  }
  const editions = book.editions;

  res.status(200).send(editions);
};

export const delete_edition = (req: Request, res: Response): void => {
  const bookID = req.params.bookID as string;
  if (!bookID) {
    res.status(400).send({ error: "bookID not present" });
    return;
  }
  const editionID = req.params.editionID as string;
  if (!editionID) {
    res.status(400).send({ error: "editionID not present" });
    return;
  }
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

  res.status(204).end();
};
