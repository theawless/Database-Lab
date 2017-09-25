import json
import os
from pprint import pprint

from pymongo import MongoClient

client = MongoClient()
db = client.youtube
videos = db.videos

for video in db.videos.find({}, {"videoInfo.snippet.title": 1, "_id": 0}):
    print(video)
