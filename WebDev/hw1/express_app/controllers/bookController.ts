import { Request, Response } from "express";
import { books, Book } from "../data";

export const add_book = (req: Request, res: Response): void => {
  const { title, subtitle, org_pub_date, tags, p_auth } = req.body;
  if (!title || !org_pub_date || !tags || !p_auth) {
    res
      .status(400)
      .json({ error: "Please provide title, org_pub_date, tags, and p_auth" });
    return;
  }
  const book = new Book(title, subtitle, org_pub_date, tags, p_auth);
  console.log(req.body);
  books.push(book);
  res.status(200).json({ book });
};

export const list_all_books = (req: Request, res: Response): void => {
  res.status(200).json(books);
};

export const list_books_by_tags = (req: Request, res: Response): void => {
  const tags = req.query.tags as string[];

  if (!tags || !Array.isArray(tags)) {
    res.status(400).json({ error: "Please provide tags as an array" });
    return;
  }

  const filteredBooks = books.filter((book) =>
    tags.every((tag) => book.tags.includes(tag)),
  );

  res.status(200).json(filteredBooks);
};

export const list_books_by_author = (req: Request, res: Response): void => {
  const id = req.params.id as string;
  if (!id) {
    res.status(400).json({ error: "Please provide an author id" });
    return;
  }
  const filteredBooks = books.filter((book) => book.p_auth.id === id);

  if (!filteredBooks) {
    res.status(404).json({ error: "Books by that author not found" });
    return;
  }

  res.status(200).json(filteredBooks);
};

export const get_book_details = (req: Request, res: Response): void => {
  const id = req.query.id;
  if (!id) {
    res.status(400).json({ error: "Please provide an author id" });
    return;
  }
  const book = books.find((book) => book.id === id);
  res.status(200).json(book);
};

export const update_book_attrs = (req: Request, res: Response): void => {
  const book_id = req.params.id;
  const update_fields: Partial<Book> = req.body;

  const book = books.find((book) => book.id === book_id);
  if (!book) {
    res.status(404).json({ error: "Book not found" });
    return;
  }

  Object.assign(book, update_fields);
  res.status(200).json(book);
};

export const delete_book = (req: Request, res: Response): void => {
  const id = req.params.id;
  const index = books.findIndex((book) => book.id === id);
  if (index === -1) {
    res.status(404).json({ error: "Book not found" });
    return;
  }
  books.splice(index, 1);

  res.status(204);
};
