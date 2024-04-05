class Author {
  name: string;
  bio: string;
  bday: string;
  genre: string;
  id: number;

  constructor(
    name: string,
    bio: string,
    bday: string,
    genre: string,
    id: number,
  ) {
    this.name = name;
    this.bio = bio;
    this.bday = bday;
    this.genre = genre;
    this.id = id;
  }
}

class Book {
  title: string;
  subtitle: string;
  org_pub_date: string;
  tags: string[];
  p_auth: Author;
  id: number;

  constructor(
    title: string,
    subtitle: string,
    org_pub_date: string,
    tags: string[],
    p_auth: Author,
    id: number,
  ) {
    this.title = title;
    this.subtitle = subtitle;
    this.org_pub_date = org_pub_date;
    this.tags = tags;
    this.p_auth = p_auth;
    this.id = id;
  }
}

class BookEd {
  ed_num: number;
  pub_date: string;
  id: number;

  constructor(ed_num: number, pub_date: string, id: number) {
    this.ed_num = ed_num;
    this.pub_date = pub_date;
    this.id = id;
  }
}

const authors: Author[] = [];
const books: Book[] = [];

export { Author, Book, BookEd, authors, books };
