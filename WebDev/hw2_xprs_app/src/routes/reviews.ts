import express, { Router } from "express";
import { auth } from "express-oauth2-jwt-bearer";
import * as reviewController from "../controllers/reviewController";
import commentRouter from "./comments";

const router: Router = express.Router();

const checkJWT = auth({
  audience: "ReviewAPI",
  issuerBaseURL: "https://dev-l8p184x0ovlzprpt.us.auth0.com/",
});
router.use(checkJWT);

router.use("/:reviewID/comments", commentRouter);

router.post("/", checkJWT, reviewController.create_review);
router.patch("/:reviewID", reviewController.patch_review_by_id);
router.get("/byAuthor/:authorID", reviewController.get_reviews_by_authorID);
router.get("/byBook/:bookID", reviewController.get_reviews_by_bookID);

export default router;
