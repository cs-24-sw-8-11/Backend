import os
import sys
sys.path.append(f'{os.path.dirname(__file__)}/../..')

from modules.database.Table import TableFactory

class Database:
    def __init__(self, path):
        factory = TableFactory(path)
        self.users = factory.create("users", [
            "id integer primary key",
            "username varchar unique not null",
            "password varchar not null",
            "userdataId integer",
            "foreign key(userdataId) references userdata(id)"
        ])
        self.journals = factory.create("journals", [
            "id integer primary key",
            "comment varchar",
            "userId integer not null",
            "foreign key(userId) references users(id)"
        ])
        self.answers = factory.create("answers", [
            "id integer primary key",
            "answer varchar not null",
            "journalId integer not null",
            "questionId integer not null",
            "foreign key(journalId) references journals(id)",
            "foreign key(questionId) references questions(id)"
        ])
        self.questions = factory.create("questions", [
            "id integer primary key",
            "tags varchar",
            "question varchar not null"
        ])
        self.settings = factory.create("settings", [
            "id integer primary key",
            "key varchar not null",
            "value varchar not null",
            "userId integer not null",
            "foreign key(userId) references users(id)"
        ])
        self.userdata = factory.create("userdata", [
            "id integer primary key",
            "agegroup integer not null",
            "occupation varchar not null",
            "userId integer not null"
        ])

if __name__ == "__main__": # some quick bad local tests
    db = Database("/tmp/db.db3")