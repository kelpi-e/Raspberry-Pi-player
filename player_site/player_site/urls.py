from django.contrib import admin
from django.urls import path, include

urlpatterns = [
    path('', include('playlists.urls', namespace='playlists')),  # namespace 'playlists'
]