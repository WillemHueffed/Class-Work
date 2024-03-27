"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.books = exports.authors = exports.BookEd = exports.Book = exports.Author = void 0;
class Author {
    constructor(name, bio, bday, genre) {
        this.name = name;
        this.bio = bio;
        this.bday = bday;
        this.genre = genre;
    }
}
exports.Author = Author;
class Book {
    constructor(title, subtitle, org_pub_date, tags, p_auth) {
        this.title = title;
        this.subtitle = subtitle;
        this.org_pub_date = org_pub_date;
        this.tags = tags;
        this.p_auth = p_auth;
    }
}
exports.Book = Book;
class BookEd {
    constructor(ed_num, pub_date) {
        this.ed_num = ed_num;
        this.pub_date = pub_date;
    }
}
exports.BookEd = BookEd;
const authors = [];
exports.authors = authors;
const books = [];
exports.books = books;
