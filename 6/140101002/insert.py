import json
import os
from py2neo import Graph, Node, Relationship, NodeSelector

# Run with py2neo version 4.0.0.b2

PATH = "/home/theawlesss/Desktop/database_lab/6/test/"
PASSWORD = "what"


def compareDescription(description1, description2):
    word_list1 = description1.split()
    word_list2 = description2.split()
    return len(set(word_list1) & set(word_list2))


def compareTags(tags1, tags2):
    return len(set(tags1) & set(tags2))


print("starting up")

arrayjson = []
filelist = os.listdir(PATH)
for i in range(len(filelist)):
    filelist[i] = PATH + filelist[i]
    page = open(filelist[i], "r")
    parsed = json.loads(page.read())
    arrayjson.append(parsed)

graph = Graph(password=PASSWORD)
transaction = graph.begin()

for i in range(len(arrayjson)):
    arraystring = arrayjson[i]['videoInfo']['statistics']
    a = Node(
        "YoutubeVideos", name=arrayjson[i]['videoInfo']['id'], commentCount=arraystring['commentCount'],
        viewCount=arraystring['viewCount'], favoriteCount=arraystring['favoriteCount'], dislikeCount=arraystring['dislikeCount'],
        likeCount=int(arraystring['likeCount'])
    )
    transaction.create(a)
    print("loading file in memory: " + str(i))

selector = NodeSelector(graph)
tnum = -1

for i in range(len(arrayjson)):
    element = arrayjson[i]
    for j in range(i - 1, -1, -1):

        if tnum >= 1000:
            print("1000 transactions in the memory")
            tnum = -1
        if tnum == -1:
            print("commit 1000 transactions")
            transaction.commit()
            transaction = graph.begin()

        if arrayjson[j]['videoInfo']['snippet']['channelId'] == element['videoInfo']['snippet']['channelId']:
            a = selector.select("YoutubeVideos").where(
                name=element['videoInfo']['id']).first()
            b = selector.select("YoutubeVideos").where(
                name=arrayjson[j]['videoInfo']['id']).first()
            channelRelation = Relationship(a, "SAME_CHANNEL", b)
            transaction.create(channelRelation)
            tnum = tnum + 1

        descriptionCount = compareDescription(
            arrayjson[i]['videoInfo']['snippet']['description'], arrayjson[j]['videoInfo']['snippet']['description'])
        if descriptionCount > 3000:
            a = selector.select("YoutubeVideos").where(
                name=element['videoInfo']['id']).first()
            b = selector.select("YoutubeVideos").where(
                name=arrayjson[j]['videoInfo']['id']).first()
            DescriptionRelation = Relationship(
                a, "SIMILAR_DESC", b, weightage=descriptionCount)
            transaction.create(DescriptionRelation)
            tnum = tnum + 1

        if 'tags' in arrayjson[i]['videoInfo']['snippet'] and 'tags' in arrayjson[j]['videoInfo']['snippet']:
            tagCount = compareTags(
                arrayjson[i]['videoInfo']['snippet']['tags'], arrayjson[j]['videoInfo']['snippet']['tags'])
            if tagCount > 20:
                a = selector.select("YoutubeVideos").where(
                    name=element['videoInfo']['id']).first()
                b = selector.select("YoutubeVideos").where(
                    name=arrayjson[j]['videoInfo']['id']).first()
                TagRelation = Relationship(
                    a, "SAME_TAGS", b, weightage=tagCount)
                transaction.create(TagRelation)
                tnum = tnum + 1

    print("loading relationship in memory: " + str(i))

transaction.commit()
print("done with insertion")
