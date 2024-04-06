import express, { Router } from "express";
import * as book_controller from "../controllers/bookController";
//import editionRouter from "./editions";
import editionRouter from "./editions";

const router: Router = express.Router();
router.use("/:bookID/editions", editionRouter);

router.post("/", book_controller.add_book);
router.get("/", book_controller.list_all_books);
router.get("/byTags/", book_controller.list_books_by_tags);
router.get("/byAuthor/:id", book_controller.list_books_by_author);

router.get("/:id", book_controller.get_book_details);
router.patch("/:id", book_controller.update_book_attrs);
router.delete("/:id", book_controller.delete_book);

export default router;
