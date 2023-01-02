/*
 Copyright (C) 2022-2023 Xu Zhen
 
 This file is part of DockbarX Xfce Panel Plugin.
 
 DockbarX Xfce Panel Plugin is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.
 
 DockbarX Xfce Panel Plugin is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this file. If not, see <http://www.gnu.org/licenses/>.
*/
#include "xfce_panel_plugin_props.h"

typedef struct {
    gint bgmode;
    gchar *color;
    gchar *image;
    gint offset;
    gint max_size;
    gchar *orient;
    gboolean expand;
    gboolean block_ah;
} PluginPropertiesPrivate;

#define PROP_BGMODE_NAME   "bgmode"
#define PROP_COLOR_NAME    "color"
#define PROP_IMAGE_NAME    "image"
#define PROP_OFFSET_NAME   "offset"
#define PROP_MAX_SIZE_NAME "max-size"
#define PROP_ORIENT_NAME   "orient"
#define PROP_EXPAND_NAME   "expand"
#define PROP_BLOCK_AH_NAME "block-ah"

enum {
    PROP_0,

    PROP_BGMODE,
    PROP_COLOR,
    PROP_IMAGE,
    PROP_OFFSET,
    PROP_MAX_SIZE,
    PROP_ORIENT,
    PROP_EXPAND,
    PROP_BLOCK_AH,

    N_PROPERTIES
};

G_DEFINE_TYPE_WITH_CODE(PluginProperties, plugin_properties, G_TYPE_OBJECT, G_ADD_PRIVATE(PluginProperties))


static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void plugin_properties_init(PluginProperties *object) {
    PluginPropertiesPrivate *priv = plugin_properties_get_instance_private(object);

    priv->bgmode = 2;
    priv->color = g_strdup("#000");
    priv->image = g_strdup("");
    priv->offset = 0;
    priv->max_size = 0;
    priv->orient = g_strdup("");
    priv->expand = FALSE;
    priv->block_ah = FALSE;
}

static void plugin_properties_finalize(GObject *object) {
    PluginPropertiesPrivate *priv = plugin_properties_get_instance_private(PLUGIN_PROPERTIES(object));

    g_free(priv->color);
    g_free(priv->image);
    g_free(priv->orient);
    G_OBJECT_CLASS(plugin_properties_parent_class)->finalize(object);
}

static void plugin_properties_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
    PluginPropertiesPrivate *priv = plugin_properties_get_instance_private(PLUGIN_PROPERTIES(object));
    switch (property_id) {
        case PROP_BGMODE:
            priv->bgmode = g_value_get_int(value);
            break;
        case PROP_COLOR:
            g_free(priv->color);
            priv->color = g_value_dup_string(value);
            break;
        case PROP_IMAGE:
            g_free(priv->image);
            priv->image = g_value_dup_string(value);
            break;
        case PROP_OFFSET:
            priv->offset = g_value_get_int(value);
            break;
        case PROP_MAX_SIZE:
            priv->max_size = g_value_get_int(value);
            break;
        case PROP_ORIENT:
            g_free(priv->orient);
            priv->orient = g_value_dup_string(value);
            break;
        case PROP_EXPAND:
            priv->expand = g_value_get_boolean(value);
            break;
        case PROP_BLOCK_AH:
            priv->block_ah = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            return;
    }
    g_object_notify_by_pspec(object, pspec);
}

static void plugin_properties_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
    PluginPropertiesPrivate *priv = plugin_properties_get_instance_private(PLUGIN_PROPERTIES(object));
    switch (property_id) {
        case PROP_BGMODE:
            g_value_set_int(value, priv->bgmode);
            break;
        case PROP_COLOR:
            g_value_set_string(value, priv->color);
            break;
        case PROP_IMAGE:
            g_value_set_string(value, priv->image);
            break;
        case PROP_OFFSET:
            g_value_set_int(value, priv->offset);
            break;
        case PROP_MAX_SIZE:
            g_value_set_int(value, priv->max_size);
            break;
        case PROP_ORIENT:
            g_value_set_string(value, priv->orient);
            break;
        case PROP_EXPAND:
            g_value_set_boolean(value, priv->expand);
            break;
        case PROP_BLOCK_AH:
            g_value_set_boolean(value, priv->block_ah);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void plugin_properties_class_init(PluginPropertiesClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->set_property = plugin_properties_set_property;
    object_class->get_property = plugin_properties_get_property;
    object_class->finalize = plugin_properties_finalize;

    obj_properties[PROP_BGMODE] = g_param_spec_int(PROP_BGMODE_NAME, NULL, NULL, 0, 2, 2, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    obj_properties[PROP_COLOR] = g_param_spec_string(PROP_COLOR_NAME, NULL, NULL, "#000", G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    obj_properties[PROP_IMAGE] = g_param_spec_string(PROP_IMAGE_NAME, NULL, NULL, "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    obj_properties[PROP_OFFSET] = g_param_spec_int(PROP_OFFSET_NAME, NULL, NULL, -32767, 32767, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    obj_properties[PROP_MAX_SIZE] = g_param_spec_int(PROP_MAX_SIZE_NAME, NULL, NULL, 0, 32767, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    obj_properties[PROP_ORIENT] = g_param_spec_string(PROP_ORIENT_NAME, NULL, NULL, "bottom", G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    obj_properties[PROP_EXPAND] = g_param_spec_boolean(PROP_EXPAND_NAME, NULL, NULL, FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    obj_properties[PROP_BLOCK_AH] = g_param_spec_boolean(PROP_BLOCK_AH_NAME, NULL, NULL, FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

gint prop_get_bgmode(GObject *object) {
    gint v;
    g_object_get(object, PROP_BGMODE_NAME, &v, NULL);
    return v;
}
void prop_set_bgmode(GObject *object, gint bgmode) {
    g_object_set(object, PROP_BGMODE_NAME, bgmode, NULL);
}

gchar *prop_get_color(GObject *object) {
    gchar *v;
    g_object_get(object, PROP_COLOR_NAME, &v, NULL);
    return v;
}
void prop_set_color(GObject *object, const gchar *color) {
    g_object_set(object, PROP_COLOR_NAME, color, NULL);
}

gchar *prop_get_image(GObject *object) {
    gchar *v;
    g_object_get(object, PROP_IMAGE_NAME, &v, NULL);
    return v;
}
void prop_set_image(GObject *object, const gchar *image) {
    g_object_set(object, PROP_IMAGE_NAME, image, NULL);
}

gint prop_get_offset(GObject *object) {
    gint v;
    g_object_get(object, PROP_OFFSET_NAME, &v, NULL);
    return v;
}
void prop_set_offset(GObject *object, gint offset) {
    g_object_set(object, PROP_OFFSET_NAME, offset, NULL);
}

gint prop_get_max_size(GObject *object) {
    gint v;
    g_object_get(object, PROP_MAX_SIZE_NAME, &v, NULL);
    return v;
}
void prop_set_max_size(GObject *object, gint max_size) {
    g_object_set(object, PROP_MAX_SIZE_NAME, max_size, NULL);
}

gchar *prop_get_orient(GObject *object) {
    gchar *v;
    g_object_get(object, PROP_ORIENT_NAME, &v, NULL);
    return v;
}
void prop_set_orient(GObject *object, const gchar *orient) {
    g_object_set(object, PROP_ORIENT_NAME, orient, NULL);
}

gboolean prop_get_expand(GObject *object) {
    gboolean v;
    g_object_get(object, PROP_EXPAND_NAME, &v, NULL);
    return v;
}
void prop_set_expand(GObject *object, gboolean expand) {
    g_object_set(object, PROP_EXPAND_NAME, expand, NULL);
}
void prop_connect_expand(GObject *object, GCallback cb, gpointer data) {
    g_signal_connect(object, "notify::"PROP_EXPAND_NAME, cb, data);
}

gboolean prop_get_block_ah(GObject *object)  {
    gboolean v;
    g_object_get(object, PROP_BLOCK_AH_NAME, &v, NULL);
    return v;
}
void prop_set_block_ah(GObject *object, gboolean block_ah) {
    g_object_set(object, PROP_BLOCK_AH_NAME, block_ah, NULL);
}
void prop_connect_block_ah(GObject *object, GCallback cb, gpointer data) {
    g_signal_connect(object, "notify::"PROP_BLOCK_AH_NAME, cb, data);
}

void prop_bind_xfconf(XfconfChannel *channel, GObject *object) {
    xfconf_g_property_bind(channel, "/mode", G_TYPE_INT, object, PROP_BGMODE_NAME);
    xfconf_g_property_bind(channel, "/color", G_TYPE_STRING, object, PROP_COLOR_NAME);
    xfconf_g_property_bind(channel, "/image", G_TYPE_STRING, object, PROP_IMAGE_NAME);
    xfconf_g_property_bind(channel, "/offset", G_TYPE_INT, object, PROP_OFFSET_NAME);
    xfconf_g_property_bind(channel, "/max-size", G_TYPE_INT, object, PROP_MAX_SIZE_NAME);
    xfconf_g_property_bind(channel, "/orient", G_TYPE_STRING, object, PROP_ORIENT_NAME);
    xfconf_g_property_bind(channel, "/expand", G_TYPE_BOOLEAN, object, PROP_EXPAND_NAME);
    xfconf_g_property_bind(channel, "/block-autohide", G_TYPE_BOOLEAN, object, PROP_BLOCK_AH_NAME);
}

