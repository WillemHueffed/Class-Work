import express, { Router } from "express";
import { requiresAuth } from "express-openid-connect";
import * as commentController from "../controllers/commentController";

const router: Router = express.Router({ mergeParams: true });

router.post("/", requiresAuth, commentController.create_comment);
router.post("/pub", commentController.create_comment);
router.delete("/:commentID", requiresAuth, commentController.delete_comment);
router.delete("/:commentIDpub", commentController.delete_comment);
router.get("/", commentController.get_comments);

export default router;
