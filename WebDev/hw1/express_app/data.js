// Refactor later?
class Author {
  constructor(name, bio, bday, genre) {
    this.name = name;
    this.bio = bio;
    this.bday = bday;
    this.genre = genre;
  }
}

class Book {
  constructor(title, subtitle, org_pub_date, tags, p_auth) {
    this.title = title;
    this.subtitle = subtitle;
    this.org_pub_date = org_pub_date;
    this.tags = tags;
    this.p_auth = p_auth;
  }
}

class BookEd {
  constructor(ed_num, pub_date) {
    this.ed_num = ed_num;
    this.pub_date = pub_date;
  }
}

let authors = [];
let books = [];

module.exports = {
  Author,
  Book,
  BookEd,
  authors,
  books,
};
