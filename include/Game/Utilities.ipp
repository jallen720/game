namespace Game
{


template<typename T>
T * array_2d_at(T * array_2d, int width, int x, int y)
{
    return &array_2d[(y * width) + x];
}


template<typename T>
void iterate_array_2d(
    T * array_2d,
    int width,
    int start_x,
    int start_y,
    int sub_width,
    int sub_height,
    bool relative_coordinates,
    const std::function<void(int, int, T &)> & callback)
{
    for (int x = start_x; x < start_x + sub_width; x++)
    {
        for (int y = start_y; y < start_y + sub_height; y++)
        {
            int x_coordinate = x;
            int y_coordinate = y;

            if (relative_coordinates)
            {
                x_coordinate -= start_x;
                y_coordinate -= start_y;
            }

            callback(x_coordinate, y_coordinate, *array_2d_at(array_2d, width, x, y));
        }
    }
}


template<typename T>
void iterate_array_2d(T * array_2d, int width, int height, const std::function<void(int, int, T &)> & callback)
{
    iterate_array_2d(array_2d, width, 0, 0, width, height, false, callback);
}


} // namespace Game
