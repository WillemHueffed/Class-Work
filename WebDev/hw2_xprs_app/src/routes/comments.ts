import express, { Router } from "express";
import { checkJWT } from "../index";
import * as commentController from "../controllers/commentController";

const router: Router = express.Router({ mergeParams: true });

router.post("/", checkJWT, commentController.create_comment);
router.delete("/:commentID", commentController.delete_comment);
router.get("/", checkJWT, commentController.get_comments);

export default router;
