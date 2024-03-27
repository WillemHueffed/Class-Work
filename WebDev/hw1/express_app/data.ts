class Author {
  name: string;
  bio: string;
  bday: string;
  genre: string;

  constructor(name: string, bio: string, bday: string, genre: string) {
    this.name = name;
    this.bio = bio;
    this.bday = bday;
    this.genre = genre;
  }
}

class Book {
  title: string;
  subtitle: string;
  org_pub_date: string;
  tags: string[];
  p_auth: Author;

  constructor(
    title: string,
    subtitle: string,
    org_pub_date: string,
    tags: string[],
    p_auth: Author,
  ) {
    this.title = title;
    this.subtitle = subtitle;
    this.org_pub_date = org_pub_date;
    this.tags = tags;
    this.p_auth = p_auth;
  }
}

class BookEd {
  ed_num: number;
  pub_date: string;

  constructor(ed_num: number, pub_date: string) {
    this.ed_num = ed_num;
    this.pub_date = pub_date;
  }
}

const authors: Author[] = [];
const books: Book[] = [];

export { Author, Book, BookEd, authors, books };
