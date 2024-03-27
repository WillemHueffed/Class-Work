import { Request, Response } from "express";

export const add_book = (req: Request, res: Response): void => {
  res.send("hello from book controller");
};

export const list_all_books = (req: Request, res: Response): void => {
  res.send("WIP");
};

export const list_book_by_tags = (req: Request, res: Response): void => {
  res.send("WIP");
};

export const list_book_by_author = (req: Request, res: Response): void => {
  res.send("WIP");
};

export const get_book_details = (req: Request, res: Response): void => {
  res.send("WIP");
};

export const update_book_attrs = (req: Request, res: Response): void => {
  res.send("WIP");
};

export const delete_book = (req: Request, res: Response): void => {
  res.send("WIP");
};

export const list_eds = (req: Request, res: Response): void => {
  res.send("WIP");
};

export const get_book_ed = (req: Request, res: Response): void => {
  res.send("WIP");
};
