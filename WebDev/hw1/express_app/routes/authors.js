var express = require("express");
var router = express.Router();
var author_controller = require("../controllers/authorController");

router.get("/", author_controller.list_authors);
router.post("/", author_controller.add_author);
router.put("/:id", author_controller.update_author_bio);

module.exports = router;
