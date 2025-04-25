/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 10:15:20 by rguigneb          #+#    #+#             */
/*   Updated: 2025/04/25 17:05:57 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "mlx.h"
#include "mlx_int.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define PI 3.1415926535

#define MAP_HEIGHT 7
#define MAP_WIDTH 10
#define SIZE 84
#define TILE_SIZE 64
#define FOV (double)(60 * (PI / 180))
#define MAX_ITERATION 10

#define SCREEN_HEIGHT MAP_HEIGHT *SIZE
#define SCREEN_WIDTH MAP_WIDTH *SIZE

#define X_SIZE 1
#define RAYS_COUNT (int)((SCREEN_WIDTH) / X_SIZE)

#define PLAYER_SPEED 2

typedef unsigned int	t_color;

typedef struct s_uvec_2
{
	unsigned int		x;
	unsigned int		y;
}						t_uvec2;

typedef struct s_vec_2
{
	int					x;
	int					y;
}						t_vec_2;

typedef struct s_fvec_2
{
	float				x;
	float				y;
}						t_fvec2;

typedef struct s_dvec_2
{
	double				x;
	double				y;
}						t_dvec2;

typedef struct s_player
{
	t_dvec2				position;
	double				pitch;
	double				rotation_angle;
}						t_player;

typedef struct s_data
{
	void				*mlx;
	void				*win;
	void				*debug_win;
	t_img				*debug_rendering_buffer;
	t_img				*rendering_buffer;
	t_player			player;
	char				*map;
	t_img				*wall_texture;
	t_img				*other;
	t_img				*floor;
	t_img				*sky;
}						t_data;

t_color	igmlx_melt_colors(t_color input, t_color filter, double filter_weight)
{
	t_color	result;
	double	input_weight;

	t_color alpha, red, green, blue;
	t_color f_alpha, f_red, f_green, f_blue;
	input_weight = 1.0 - filter_weight;
	alpha = (input >> 24) & 0xFF;
	red = (input >> 16) & 0xFF;
	green = (input >> 8) & 0xFF;
	blue = input & 0xFF;
	f_alpha = (filter >> 24) & 0xFF;
	f_red = (filter >> 16) & 0xFF;
	f_green = (filter >> 8) & 0xFF;
	f_blue = filter & 0xFF;
	alpha = (alpha * input_weight) + (f_alpha * filter_weight);
	red = (red * input_weight) + (f_red * filter_weight);
	green = (green * input_weight) + (f_green * filter_weight);
	blue = (blue * input_weight) + (f_blue * filter_weight);
	result = ((t_color)alpha << 24) | ((t_color)red << 16) | ((t_color)green << 8) | (t_color)blue;
	return (result);
}

void	switch_wall_texture(t_data *data)
{
	t_img	*tmp;

	tmp = data->wall_texture;
	data->wall_texture = data->other;
	data->other = tmp;
}

double	normalize_angle(double angle)
{
	angle = fmod(angle, (2.0f * PI));
	if (angle < 0)
		angle += (2.0f * PI);
	return (angle);
}

void	on_key_pressed(int key, t_data *data)
{
	double	adj;
	double	opo;

	printf("key :%d\n", key);
	if (key == 'b')
		switch_wall_texture(data);
	if (key == 'w')
	{
		adj = cos(data->player.rotation_angle) * 3.5;
		opo = sin(data->player.rotation_angle) * 3.5;
		data->player.position.x += adj;
		data->player.position.y += opo;
	}
	else if (key == 's')
	{
		adj = cos(data->player.rotation_angle) * 3.5;
		opo = sin(data->player.rotation_angle) * 3.5;
		data->player.position.x -= adj;
		data->player.position.y -= opo;
	}
	if (key == 65362)
	{
		data->player.pitch += 10;
	}
	else if (key == 65364)
	{
		data->player.pitch -= 10;
	}
	if (key == 'a')
	{
		data->player.rotation_angle -= 0.05;
	}
	else if (key == 'd')
	{
		data->player.rotation_angle += 0.05;
	}
	data->player.rotation_angle = normalize_angle(data->player.rotation_angle);
}


double	distance_between(t_dvec2 vec1, t_dvec2 vec2)
{
	double	mx;
	double	my;

	mx = vec2.x - vec1.x;
	my = vec2.y - vec1.y;
	return (sqrt(mx * mx + my * my));
}

#define DISTANCE_FROM_CAMERA (double)((SCREEN_WIDTH / 2) / tan(FOV / 2))

double	ft_max(double a, double b)
{
	if (a > b)
		return (a);
	return (b);
}

void	put_pixel_to_buffer(t_img *buffer, t_uvec2 pos, int color)
{
	if (pos.x < 0)
		pos.x = 0;
	if (pos.x > SCREEN_WIDTH)
		pos.x = SCREEN_WIDTH;
	if (pos.y < 0)
		pos.y = 0;
	if (pos.y > SCREEN_HEIGHT)
		pos.y = SCREEN_HEIGHT;
	((unsigned int *)buffer->data)[(pos.y * (buffer->size_line / 4))
		+ pos.x] = color;
}

void	draw_line(t_img *buffer, int color, t_vec_2 start, t_vec_2 end)
{
	double	distance_x;
	double	distance_y;
	double	step;
	double	stepX;
	double	stepY;
	int		i;
	t_vec_2	tmp;
	int		y;
	int		x;

	i = 0;
	if (end.y < 0 && abs(end.y - start.y) > abs(end.x - start.x))
	{
		tmp = start;
		start = end;
		end = tmp;
	}
	distance_x = end.x - start.x;
	distance_y = end.y - start.y;
	step = fmax(fabs(distance_x), fabs(distance_y));
	if (step != 0)
	{
		stepX = distance_x / step;
		stepY = distance_y / step;
	}
	while (i <= step + 1)
	{
		y = floor(start.y + i * stepY);
		x = floor(start.x + i * stepX);
		put_pixel_to_buffer(buffer, (t_uvec2){x, y}, color);
		i++;
	}
}

void	draw_rect(t_img *buffer, int color, t_uvec2 start, t_uvec2 end)
{
	t_uvec2	tmp;

	tmp = start;
	while (tmp.x < end.x)
	{
		tmp.y = start.y;
		while (tmp.y < end.y)
		{
			put_pixel_to_buffer(buffer, tmp, color);
			tmp.y++;
		}
		tmp.x++;
	}
}

void	draw_straight_line(t_img *buffer, t_data *data, t_dvec2 ray, double angle, int color,
		int x, double height, double distance)
{
	int		y;
	int		t;
	int		textureX;
	t_color	dcolor;
	double	darkness_weight;
	int		texelX;
	double	distanceFromTop;
	int		texelY;
	float	dist;
	double	currentFloorX;
	double	currentFloorY;
	int		floorTexX;
	int		floorTexY;
	float	rayDirX;
	float	rayDirY;
	int		checkerBoardPattern;
		int floorTexture;

	t = 0;
	while (t < (SCREEN_HEIGHT - height) / 2)
	{
		dcolor = ((t_color *)(data->sky->data))[(data->sky->size_line / 4) * t
			+ (int)((float)x + (data->player.rotation_angle * 1000)) % (SCREEN_WIDTH)];
		put_pixel_to_buffer(buffer, (t_uvec2){x, t},
			(int)dcolor);
		t++;
	}
	// y = t;
	darkness_weight = (150 / distance);
	if (darkness_weight < 0)
		darkness_weight = 0;
	if (darkness_weight > 1)
		darkness_weight = 1;
	if (color != 0x00EE00)
		texelX = (int)floor(ray.y) % TILE_SIZE;
	else
		texelX = (int)floor(ray.x) % TILE_SIZE;
	// Scale texelX to the texture width
	texelX = (texelX * data->wall_texture->width) / TILE_SIZE;
	// color = igmlx_melt_colors(0x000000, color, darkness_weight);
	y = t;
	while (y < SCREEN_HEIGHT - t)
	{
		distanceFromTop = y + (height / 2) - (SCREEN_HEIGHT / 2);
		texelY = (1LL * distanceFromTop * data->wall_texture->height + height
				/ 2) / height;
		// Ensure texelY stays within the texture height bounds
		texelY = texelY % data->wall_texture->height;
		dcolor = ((t_color *)(data->wall_texture->data))[(texelY
				* (data->wall_texture->size_line / 4)) + texelX];
		dcolor = igmlx_melt_colors(0x000000, dcolor, darkness_weight);
		put_pixel_to_buffer(buffer, (t_uvec2){x, y + data->player.pitch},
			(int)dcolor);
		y++;
	}

	double dirX = cos(data->player.rotation_angle);
	double dirY = sin(data->player.rotation_angle);

	// camera plane perpendicular to direction (FOV usually about 66 degrees)
	double planeX = -dirY * 0.66f;
	double planeY =  dirX * 0.66f;

	double cameraX = 2 * x / (double)SCREEN_WIDTH - 1;

	rayDirX = dirX + planeX * cameraX;
	rayDirY = dirY + planeY * cameraX;

	//calculate value of wallX
	double wallX, wallY; //where exactly the wall was hit

	int mapX = (int)ray.x;
	int mapY = (int)ray.y;

	double floorXWall, floorYWall; //x, y position of the floor texel at the bottom of the wall
	if (color != 0x00EE00) wallX = (int)ray.y + distance * rayDirY;
	else           wallX = (int)ray.x + distance * rayDirX;
	wallX -= floor((wallX));
      //4 different wall directions possible
      if(color != 0x00EE00 && rayDirX > 0)
      {
        floorXWall = mapX;
        floorYWall = mapY + wallX;
      }
      else if(color != 0x00EE00 && rayDirX < 0)
      {
        floorXWall = mapX + 1.0;
        floorYWall = mapY + wallX;
      }
      else if(color != 0x00EE00 && rayDirY > 0)
      {
        floorXWall = mapX + wallX;
        floorYWall = mapY;
      }
      else
      {
        floorXWall = mapX + wallX;
        floorYWall = mapY + 1.0;
      }

	while (y < SCREEN_HEIGHT)
	{
		double currentDist = SCREEN_HEIGHT / (2.0 * y - SCREEN_HEIGHT);
		double distPlayer = 0;
		double weight = (currentDist - (distPlayer)) / ((distance) - distPlayer);

		currentFloorX = weight * floorXWall + (1.0 - weight) * (data->player.position.x);
		currentFloorY = weight * floorYWall + (1.0 - weight) * (data->player.position.y);


		int floorTexX, floorTexY;

		floorTexX = (int)(currentFloorX * 64.f) % 64;
		floorTexY = (int)(currentFloorY * 64.f) % 64;

		dcolor = ((t_color *)(data->floor->data))[(floorTexY * (data->floor->size_line / 4))
			+ floorTexX];
		put_pixel_to_buffer(buffer, (t_uvec2){x, y + data->player.pitch},
			(int)dcolor);
		y++;
	}
	// int XWall, YWall;
	// rayDirX = ray.x - data->player.position.x;
	// rayDirY = ray.y - data->player.position.y;
	// if (color != 0x00EE00)
	// {
	// 	// Wall was hit on X-axis (vertical wall)
	// 	XWall = ray.x / TILE_SIZE;
	// 	YWall = data->player.position.y + distance * rayDirY;
	// }
	// else
	// {
	// 	// Wall was hit on Y-axis (horizontal wall)
	// 	XWall = data->player.position.x + distance * rayDirX;
	// 	YWall = ray.y / TILE_SIZE;
	// 	;
	// }
	// while (y < SCREEN_HEIGHT)
	// {
	// 	dist = height / (2 * y - height);
	// 	weight = dist / distance;
	// 	currentFloorX = weight * XWall + (1 - weight) * data->player.position.x;
	// 	currentFloorY = weight * YWall + (1 - weight) * data->player.position.y;
	// 	floorTexX = (int)(currentFloorX * 64) % 64;
	// 	floorTexY = (int)(currentFloorY * 64) % 64;
	// 	dcolor = ((t_color *)(data->wall_texture->data))[(floorTexY
	// 			* (data->wall_texture->size_line / 4)) + floorTexX];
	// 	put_pixel_to_buffer(buffer, (t_uvec2){x, y}, (int)dcolor);
	// 	y++;
	// }
	// y = t;
	// while (y < SCREEN_HEIGHT - t)
	// {
	// 	put_pixel_to_buffer(buffer, (t_uvec2){x + 1, y}, color);
	// 	y++;
	// }
	// y = t;
	// while (y < SCREEN_HEIGHT - t)
	// {
	// 	put_pixel_to_buffer(buffer, (t_uvec2){x + 2, y}, color);
	// 	y++;
	// }
	// y = t;
	// while (y < SCREEN_HEIGHT - t)
	// {
	// 	put_pixel_to_buffer(buffer, (t_uvec2){x + 3, y}, color);
	// 	y++;
	// }
}

void	draw_map(t_img *buffer, char *map)
{
	t_uvec2	tmp;

	tmp = (t_uvec2){0, 0};
	while (tmp.x < MAP_WIDTH)
	{
		tmp.y = 0;
		while (tmp.y < MAP_HEIGHT)
		{
			if (map[tmp.y * MAP_WIDTH + tmp.x] == 1)
				draw_rect(buffer, 0x00FFFF, (t_uvec2){tmp.x * SIZE, tmp.y
					* SIZE}, (t_uvec2){tmp.x * SIZE + SIZE - 2, tmp.y * SIZE
					+ SIZE - 2});
			else
				draw_rect(buffer, 0xFFFFFF, (t_uvec2){tmp.x * SIZE, tmp.y
					* SIZE}, (t_uvec2){tmp.x * SIZE + SIZE - 2, tmp.y * SIZE
					+ SIZE - 2});
			tmp.y++;
		}
		tmp.x++;
	}
}

bool	is_wall(char *map, t_vec_2 pos)
{
	if (pos.x < 0 || pos.x >= SCREEN_WIDTH || pos.y < 0
		|| pos.y >= SCREEN_HEIGHT)
		return (true);
	return (map[(pos.y / SIZE) * MAP_WIDTH + (pos.x / SIZE)] == 1);
}

bool	is_looking_bottom_right(double angle)
{
	return (angle >= 0 && angle <= PI / 2);
}

bool	is_looking_bottom_left(double angle)
{
	return (angle >= PI / 2 && angle <= PI);
}

bool	is_looking_top(double angle)
{
	return (angle <= PI && angle >= 0);
}

bool	is_looking_left(double angle)
{
	return (angle <= PI + (PI / 2) && angle >= PI / 2);
}

bool	is_looking_right(double angle)
{
	return (angle >= 0 && angle <= PI / 2 || angle >= PI + (PI / 2)
		&& angle <= 2 * PI);
}

bool	is_looking_bottom(double angle)
{
	return (angle >= PI && angle <= PI * 2);
}

bool	is_looking_top_left(double angle)
{
	return (angle >= PI && angle <= PI + PI / 2);
}

bool	is_looking_top_right(double angle)
{
	return (angle >= PI + PI / 2 && angle <= 2 * PI);
}

t_dvec2	get_horizontal_colision(t_img *buffer, t_dvec2 start, char *map,
		double angle)
{
	int		i;
	t_dvec2	A;
	double	Ya;
	double	Xa;

	i = 0;
	if (is_looking_bottom(angle))
		A.y = floor((int)floor(start.y) / SIZE) * SIZE;
	else
		A.y = floor((int)floor(start.y) / SIZE) * SIZE + SIZE;
	if (is_looking_bottom(angle))
		Ya = -SIZE;
	else
		Ya = SIZE;
	A.x = ((A.y - start.y) / tan(angle)) + start.x;
	if (is_looking_bottom(angle))
		A.y -= 0.001f;
	Xa = Ya / tan(angle);
	while (i < MAX_ITERATION && A.y / SIZE >= 0 && A.y / SIZE <= MAP_HEIGHT)
	{
		if (is_wall(map, (t_vec_2){(int)floor(A.x), (int)floor(A.y)}))
			break ;
		A.x += Xa;
		A.y += Ya;
		i++;
	}
	return (A);
}

t_dvec2	get_vertical_colision(t_img *buffer, t_dvec2 start, char *map,
		double angle)
{
	int		i;
	t_dvec2	B;
	double	Xa;
	double	Ya;

	i = 0;
	if (is_looking_left(angle))
		B.x = floor((int)floor(start.x) / SIZE) * SIZE;
	else
		B.x = floor((int)floor(start.x) / SIZE) * SIZE + SIZE;
	B.y = start.y + (start.x - B.x) * -tan(angle);
	if (is_looking_left(angle))
		Xa = -SIZE;
	else
		Xa = SIZE;
	Ya = Xa * tan(angle);
	if (is_looking_left(angle))
		B.x -= 0.01f;
	while (i < MAX_ITERATION && B.x / SIZE >= 0 && B.x / SIZE <= MAP_WIDTH)
	{
		if (is_wall(map, (t_vec_2){(int)floor(B.x), (int)floor(B.y)}))
			break ;
		B.x += Xa;
		B.y += Ya;
		i++;
	}
	B.x -= 0.001f;
	return (B);
}

void	draw_ray(t_img *buffer, t_player player, t_data *data, double angle,
		int i)
{
	t_dvec2	vert;
	t_dvec2	hori;
	t_dvec2	pos;
	double	height;
	double	distance;
	double	r;
	int		color;
	double	a;
	double	weight;

	vert = get_vertical_colision(data->rendering_buffer, player.position,
			data->map, angle);
	hori = get_horizontal_colision(data->rendering_buffer, player.position,
			data->map, angle);
	// draw_rect(buffer, 0xDDDD00, (t_uvec2){vert.x - 2, vert.y - 2},
	// 	(t_uvec2){vert.x + 2, vert.y + 2});
	// draw_rect(buffer, 0x0000DD, (t_uvec2){hori.x - 2, hori.y - 2},
	// 	(t_uvec2){hori.x + 2, hori.y + 2});
	if (distance_between(player.position,
			hori) < distance_between(player.position, vert))
	{
		pos = hori;
		color = 0x00EE00;
	}
	else
	{
		pos = vert;
		color = 0x00DD00;
	}
	draw_line(data->debug_rendering_buffer, color,
		(t_vec_2){(int)floor(player.position.x), (int)floor(player.position.y)},
		(t_vec_2){(int)floor(pos.x), (int)floor(pos.y)});
	a = fabs(player.rotation_angle - angle);
	distance = distance_between(player.position, pos);
	distance *= cos(a);
	// pos.y += player.pitch;
	height = ((double)(64 / distance) * DISTANCE_FROM_CAMERA);
	// color = (color >> 1) & 8355711;
	draw_straight_line(buffer, data, pos, angle, color, i, height, distance);
}

void	raycaster(t_img *buffer, t_player player, double angle, int len)
{
	draw_line(buffer, 0xFF00FF, (t_vec_2){player.position.x, player.position.y},
		(t_vec_2){player.position.x + cos(angle) * len, player.position.y
		+ sin(angle) * len});
}

void	draw_player(t_img *buffer, t_data *data, t_player player)
{
	int		i;
	double	angle;
	int		x;

	draw_rect(data->debug_rendering_buffer, 0xFF0000,
		(t_uvec2){((int)player.position.x) - 5, ((int)player.position.y) - 5},
		(t_uvec2){((int)player.position.x) + 5, ((int)player.position.y) + 5});
	i = 0;
	x = 0;
	angle = normalize_angle(player.rotation_angle - (FOV / 2));
	while (i < RAYS_COUNT)
	{
		draw_ray(buffer, player, data, normalize_angle(angle), x);
		i += 1;
		x += X_SIZE;
		angle += (double)(FOV / RAYS_COUNT);
	}
	raycaster(data->debug_rendering_buffer, player, player.rotation_angle, 15);
}

void	draw_sky(t_data *data)
{
	// int x = 0;
	// int y = 0;
	// t_color color;

	// while (y < SCREEN_HEIGHT / 2)
	// {
	// 	x = 0;
	// 	while (x < SCREEN_WIDTH)
	// 	{
	// 		c
	// 		put_pixel_to_buffer(data->rendering_buffer, (t_uvec2){x, y},
	// 			(int)color);
	// 		x++;
	// 	}
	// 	y++;
	// }
	// draw_rect(data->rendering_buffer, 0x00F0EE, (t_uvec2){0, 0},
	// 	(t_uvec2){SCREEN_WIDTH, SCREEN_HEIGHT / 2 + data->player.pitch});
}

#define CAMERA_Z (float)(0.5f * SCREEN_HEIGHT)

void	draw_floor(t_data *data)
{
	t_dvec2	direction;
	t_dvec2	plane;
	t_dvec2	position;

	// direction = (t_dvec2){cos(data->player.rotation_angle),
	// 	sin(data->player.rotation_angle)};
	// plane = (t_dvec2){-direction.y, direction.x * 0.66f};
	// position = {floor(data->player.position.x / 64),
	// 	floor(data->player.position.y / 64)};
}

void	loop(t_data *data)
{
	usleep(10000);
	mlx_destroy_image(data->mlx, data->rendering_buffer);
	mlx_destroy_image(data->mlx, data->debug_rendering_buffer);
	// printf("distance : %d\n", get_distance_from_the_camera(data));
	data->rendering_buffer = mlx_new_image(data->mlx, SCREEN_WIDTH,
			SCREEN_HEIGHT);
	data->debug_rendering_buffer = mlx_new_image(data->mlx, SCREEN_WIDTH,
			SCREEN_HEIGHT);
	// draw_line(data->rendering_buffer, 0xFF0000, (t_uvec2){50, 200},
	// (t_uvec2){0,
	// 	0});
	// draw_line(data->rendering_buffer, 0xDD00DD, (t_uvec2){0, 0},
	// (t_uvec2){50,
	// 	90});
	// draw_rect(data->rendering_buffer, 0x00DF0F, (t_uvec2){90, 90},
	// 	(t_uvec2){120, 99});
	draw_map(data->debug_rendering_buffer, data->map);
	draw_player(data->rendering_buffer, data, data->player);
	mlx_put_image_to_window(data->mlx, data->win, data->rendering_buffer, 0, 0);
	mlx_put_image_to_window(data->mlx, data->debug_win,
		data->debug_rendering_buffer, 0, 0);
}

int	destroy_close(t_data *data)
{
	mlx_loop_end(data->mlx);
	return (0);
}

int	main(int argc, char const *argv[])
{
	t_data	data;
	char	map[MAP_HEIGHT * MAP_WIDTH] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0,
			0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int		d;

	bzero(&data, sizeof(t_data));
	data.player.position.x = SCREEN_WIDTH / 2 - SIZE;
	data.player.position.y = SCREEN_HEIGHT / 2 - SIZE;
	data.player.rotation_angle = 45 * (PI / 180);
	data.map = (char *)&map;
	data.mlx = mlx_init();
	if (!data.mlx)
		return (EXIT_FAILURE);
	data.win = mlx_new_window(data.mlx, SCREEN_WIDTH, SCREEN_HEIGHT,
			"RayCaster in C");
	data.debug_win = mlx_new_window(data.mlx, SCREEN_WIDTH, SCREEN_HEIGHT,
			"DEBUG RayCaster in C");
	if (!data.win)
		return (mlx_destroy_display(data.mlx), free(data.mlx), EXIT_FAILURE);
	data.debug_rendering_buffer = mlx_new_image(data.mlx, SCREEN_WIDTH,
			SCREEN_HEIGHT);
	data.rendering_buffer = mlx_new_image(data.mlx, SCREEN_WIDTH,
			SCREEN_HEIGHT);
	mlx_hook(data.win, DestroyNotify, 0, destroy_close, &data.mlx);
	mlx_hook(data.win, KeyPress, KeyPressMask, (int (*)())on_key_pressed,
		&data);
	mlx_hook(data.debug_win, DestroyNotify, 0, destroy_close, &data.mlx);
	mlx_hook(data.debug_win, KeyPress, KeyPressMask, (int (*)())on_key_pressed,
		&data);
	data.wall_texture = mlx_xpm_file_to_image(data.mlx, "assets/wall1.xpm", &d,
			&d);
	data.other = mlx_xpm_file_to_image(data.mlx, "assets/romain.xpm", &d, &d);
	data.floor = mlx_xpm_file_to_image(data.mlx, "assets/wall2.xpm", &d, &d);
	data.sky = mlx_xpm_file_to_image(data.mlx, "assets/sky.xpm", &d, &d);
	// mlx_key_hook(data.win, (int (*)(void *))on_key_pressed, &data);
	mlx_loop_hook(data.mlx, (int (*)(void *))loop, &data);
	mlx_loop(data.mlx);
	mlx_destroy_image(data.mlx, data.rendering_buffer);
	mlx_destroy_window(data.mlx, data.win);
	mlx_destroy_display(data.mlx);
	free(data.mlx);
	return (0);
}

// char map[MAP_HEIGHT * MAP_WIDTH] = {
// 	1, 1, 1, 1, 1,
// 	1, 0, 0, 0, 1,
// 	1, 0, 0, 0, 1,
// 	1, 0, 0, 0, 1,
// 	1, 1, 1, 1, 1,
// };

// char map[MAP_HEIGHT * MAP_WIDTH] = {
// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
// };
