import express, { Router } from "express";
import { auth } from "express-oauth2-jwt-bearer";
import * as commentController from "../controllers/commentController";

const router: Router = express.Router({ mergeParams: true });

const checkJWT = auth({
  audience: "ReviewAPI",
  issuerBaseURL: "https://dev-l8p184x0ovlzprpt.us.auth0.com/",
});

router.use(checkJWT);

router.post("/", commentController.create_comment);
router.delete("/:commentID", commentController.delete_comment);
router.get("/", checkJWT, commentController.get_comments);

export default router;
