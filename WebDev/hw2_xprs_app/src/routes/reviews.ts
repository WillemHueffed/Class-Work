import express, { Router } from "express";
import * as reviewController from "../controllers/reviewController";
import commentRouter from "./comments";

const router: Router = express.Router();

router.use("/:reviewID/comments", commentRouter);

router.post("/", reviewController.create_review);
router.patch("/:reviewID", reviewController.patch_review_by_id);
router.get("/byAuthor/:authorID", reviewController.get_reviews_by_authorID);
router.get("/byBook/:bookID", reviewController.get_reviews_by_bookID);

export default router;
