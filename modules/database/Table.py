from sqlite3 import Cursor, Connection

import json

# this file is basically just to abstract as much as possible from the underlying SQL implementation

class Table:
    def __init__(self, name, columns, conn) -> None:
        self.connection:Connection = conn
        self.name:str = name
        self.columns:list[str] = columns
    def __len__(self):
        cursor = Cursor(self.connection)
        cursor.execute(f"select * from {self.name}")
        size = len(cursor.fetchall())
        cursor.close()
        return size
    
    def add(self, *args, cols:list[str] = None):
        cursor = Cursor(self.connection)
        cols = cols if cols is not None else [col.split(" ")[0] for col in self.columns if not "primary key" in col and not "foreign key" in col and "not null" in col]
        cursor.execute(f"insert into {self.name} ({', '.join(cols)}) values (?{' '.join(', ?' for _ in range(len(cols)-1))})", [json.dumps(arg) for arg in args])
        cursor.close()
        self.connection.commit()

class TableFactory: # basically a fancy way to say database
    def __init__(self, path):
        self.connection = Connection(path)
    
    def create(self, name:str, columns:list[str]) -> Table:
        t = Table(name, columns, self.connection)
        cursor = Cursor(self.connection)
        cursor.execute(f"create table if not exists {name} ({', '.join(columns)})")
        cursor.close()
        self.connection.commit()
        return t