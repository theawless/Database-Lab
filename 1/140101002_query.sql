#1
select c.cname
from Course c
where c.cid = 'CS344';

#2
select s.sname
from Student s
where s.sname like 'Q%';

#3
select s.sid, s.sname
from Student s, Registration r
where s.sid = r.sid
group by s.sid, s.sname
having count(s.sid) >= 3;

#4
select s.sphone
from Student s, Course c, Registration r
where r.sid = s.sid and r.cid = c.cid and c.cid = 'CS344';

#5
select avg(r.grade * c.credit) as cpi
from Course c, Registration r
where r.sid = 1 and r.cid = c.cid;

#6
select r.sid
from Course c, Registration r
where r.cid = c.cid
group by r.sid
having avg(r.grade * c.credit) > 7.5;

#7
select r.sid
from Course c, Registration r
where r.cid = c.cid
group by r.sid
having avg(r.grade * c.credit) >= all(select avg(r.grade * c.credit)
                                      from Course c, Registration r
                                      where r.cid = c.cid
                                      group by r.sid);

#8
select c.cid, c.cname
from Course c, Registration r
where c.cid = r.cid
group by c.cid, c.cname
having count(r.cid) = (select count(s.sid)
                       from Student s);

#9
select c.cid, avg(r.grade)
from Course c, Registration r
where c.cid = r.cid
group by c.cid;

#10
select distinct s.sid
from Student s, Registration r
where s.sid = r.sid and r.grade >= all(select avg(r.grade)
                                       from Course c, Registration r
                                       where c.cid = r.cid);
                                       
