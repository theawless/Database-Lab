# creation

create table Course (cid char(5), cname varchar(255), credit decimal(1,0), primary key(cid));

create table Student (sid int, sname char(5), sphone decimal (10,0), primary key(sid), key(sphone));

create table Registration (sid int, cid char(5), grade decimal(1,0), primary key(sid, cid), foreign key(sid) references Student(sid), foreign key(cid) references Course(cid));

