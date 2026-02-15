from django.urls import path
from . import views

app_name = 'playlists'

urlpatterns = [
    path('', views.index, name='index'),
    path('create/', views.playlist_create, name='playlist_create'),
    path('covers/<path:path>/', views.serve_cover, name='serve_cover'),
    path('api/extract/', views.api_extract_metadata, name='api_extract'),
    path('<str:playlist_id>/save/', views.playlist_save, name='playlist_save'),
    path('<str:playlist_id>/add_track/', views.playlist_add_track, name='playlist_add_track'),
    path('<str:playlist_id>/remove_track/', views.playlist_remove_track, name='playlist_remove_track'),
    path('<str:playlist_id>/delete/', views.playlist_delete, name='playlist_delete'),
    path('<str:playlist_id>/', views.playlist_edit, name='playlist_edit'),  # самый общий – в конце
]