create table person(id int primary key, name varchar);
insert into person values(1,'Bob');
insert into person values(2,'Jane');
insert into person values(3,'Dave');
insert into person values(4,'Heidi');
insert into person values(5,'Hobo');
insert into person values(6,'Alice');

create table pet(name varchar, owner int, foreign key(owner) references person(id));
insert into pet values ('Tiger', 0);
insert into pet values ('Spot', 1);
insert into pet values ('Marco', 1);
insert into pet values ('Steve the Fish', 4);

select * from person;
select * from pet;

select person.name, pet.name from person inner join pet on person.id = pet.owner where person.name = 'Bob';