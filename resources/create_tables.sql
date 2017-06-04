/**
 * Author:  xavier
 * Created: 14-May-2017
 */
DROP TABLE IF EXISTS osm2gadm_polygons;
CREATE TABLE osm2gadm_polygons
(
  id serial,
  name varchar (128),
  iso2 char (2),
  relation_id bigint,
  geom geometry(Polygon, 4326),
  max_x double precision,
  min_x double precision,
  max_y double precision,  
  min_y double precision,
  CONSTRAINT osm2gadm_polygons_pkey PRIMARY KEY (id),
  CONSTRAINT osm2gadm_polygons_relation_id_fkey FOREIGN KEY (relation_id) REFERENCES relations (id)
);

DROP TABLE IF EXISTS osm2gadm_lines;
CREATE TABLE osm2gadm_lines
(
  id serial,
  relation_id bigint,
  type varchar (16),
  geom geometry (Linestring, 4326),
  CONSTRAINT osm2gadm_lines_pkey PRIMARY KEY (id),
  CONSTRAINT osm2gadm_lines_relation_id_fkey FOREIGN KEY (relation_id) REFERENCES relations (id)
);
