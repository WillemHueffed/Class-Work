import express, { Router } from "express";
import * as book_controller from "../controllers/bookController";

const router: Router = express.Router();

router.post("/");
router.get("/", book_controller.add_book);

export default router;
