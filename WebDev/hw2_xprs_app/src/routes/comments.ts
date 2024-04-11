import express, { Router } from "express";
import * as commentController from "../controllers/commentController";

const router: Router = express.Router({ mergeParams: true });

router.post("/", commentController.create_comment);
router.delete("/:commentID", commentController.delete_comment);
router.get("/", commentController.get_comments);

export default router;
