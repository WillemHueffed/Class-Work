const book_controller = require("../controllers/bookController");
var express = require("express");
var router = express.Router();

router.post("/");
router.get("/", book_controller.add_book);

module.exports = router;
