import json
import os
from py2neo import Graph, Node, Relationship, NodeSelector


def compareDescription(description1, description2):
    word_description1 = description1.split()
    word_description2 = description2.split()
    count = len(set(word_description2) & set(word_description1))
    return count


def compareTags(tags1, tags2):
    return len(set(tags1) & set(tags2))


print("starting up")

arrayjson = []
filelist = os.listdir("/home/theawlesss/Desktop/database_lab/6/test/")
for i in range(len(filelist)):
    filelist[i] = "/home/theawlesss/Desktop/database_lab/6/test/" + filelist[i]
    page = open(filelist[i], "r")
    parsed = json.loads(page.read())
    arrayjson.append(parsed)

graph = Graph(password="what")
transaction = graph.begin()

for i in range(len(arrayjson)):
    arraystring = arrayjson[i]['videoInfo']['statistics']
    a = Node(
        "Youtube", name=arrayjson[i]['videoInfo']['id'], commentCount=arraystring['commentCount'],
        viewCount=arraystring['viewCount'], favoriteCount=arraystring['favoriteCount'], dislikeCount=arraystring['dislikeCount'],
        likeCount=int(arraystring['likeCount'])
    )
    transaction.create(a)
    print("doing something with " + str(i))

transaction.commit()
selector = NodeSelector(graph)
transaction = graph.begin()

for i in range(len(arrayjson)):
    element = arrayjson[i]
    for j in range(i - 1, -1, -1):
        if arrayjson[j]['videoInfo']['snippet']['channelId'] == element['videoInfo']['snippet']['channelId']:
            a = selector.select("Youtube", property_key='name',
                                property_value=element['videoInfo']['id'])
            b = selector.select("Youtube", property_key='name',
                                property_value=arrayjson[j]['videoInfo']['id'])
            channelRelation = Relationship(a, "Same Channel", b)
            transaction.create(channelRelation)

        descriptionCount = compareDescription(
            arrayjson[i]['videoInfo']['snippet']['description'], arrayjson[j]['videoInfo']['snippet']['description'])
        if descriptionCount != 0:
            a = selector.select("Youtube", property_key='name',
                                property_value=element['videoInfo']['id'])
            b = selector.select("Youtube", property_key='name',
                                property_value=arrayjson[j]['videoInfo']['id'])
            DescriptionRelation = Relationship(
                a, "Matching Description", b, weightage=descriptionCount)
            transaction.create(DescriptionRelation)

        if 'tags' in arrayjson[i]['videoInfo']['snippet'] and 'tags' in arrayjson[j]['videoInfo']['snippet']:
            tagCount = compareTags(
                arrayjson[i]['videoInfo']['snippet']['tags'], arrayjson[j]['videoInfo']['snippet']['tags'])
            if tagCount != 0:
                a = selector.select(
                    "Youtube", property_key='name', property_value=element['videoInfo']['id'])
                b = selector.select(
                    "Youtube", property_key='name', property_value=arrayjson[j]['videoInfo']['id'])
                TagRelation = Relationship(
                    a, "Matching Tags", b, weightage=tagCount)
                transaction.create(TagRelation)
    print("doing something with" + str(i))

transaction.commit()
print("done")
