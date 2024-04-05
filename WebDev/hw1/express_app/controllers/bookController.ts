import { Request, Response } from "express";

export const add_book = (req: Request, res: Response): void => {
  res.send("hello from book controller");
};

export const list_all_books = (req: Request, res: Response): void => {
  res.send("WIP 1");
};

export const list_books_by_tags = (req: Request, res: Response): void => {
  res.send("WIP 2");
};

export const list_books_by_author = (req: Request, res: Response): void => {
  res.send("WIP 3");
};

export const get_book_details = (req: Request, res: Response): void => {
  res.send("WIP 4");
};

export const update_book_attrs = (req: Request, res: Response): void => {
  res.send("WIP 5 ");
};

export const delete_book = (req: Request, res: Response): void => {
  res.send("WIP 6");
};
