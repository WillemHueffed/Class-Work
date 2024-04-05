import express, { Router } from "express";
import * as book_controller from "../controllers/bookController";
//import editionRouter from "./editions";
import editionRouter from "./editions";

const router: Router = express.Router();
router.use("/:bookID/collections", editionRouter);

router.post("/", book_controller.add_book);
router.get("/", book_controller.list_all_books);
router.get("/byTags/", book_controller.list_books_by_tags);
router.get("/byAuthor/authorID", book_controller.list_books_by_author);

router.get("/:bookID", book_controller.get_book_details);
router.patch("/:bookID", book_controller.update_book_attrs);
router.delete("/:bookID", book_controller.delete_book);

export default router;
