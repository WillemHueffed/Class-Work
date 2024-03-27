import express, { Router } from "express";
import * as author_controller from "../controllers/authorController";

const router: Router = express.Router();

router.get("/", author_controller.list_authors);
router.post("/", author_controller.add_author);
router.put("/:id", author_controller.update_author_bio);

export default router;
