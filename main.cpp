#include "stdafx.h"
#include "camera.h"
#include "interpolation.h"
#include <glut.h>
#include <vector>
#include <functional>
#include <math.h>

using key_frames_t = std::vector<glm::mat4>;
using path_t = std::vector<glm::mat4>;
using intepolator_t = std::function<glm::mat4(glm::mat4, glm::mat4, float)>;
using custom_func_t = std::function<double(double)>;

std::vector<std::vector<float>> colors
{
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f }
};

std::vector<glm::vec3> objects
{
    { 0.0f, 0.0f, -4.0f },
    { 4.0f, 0.0f, -4.0f },
    { 8.0f, 0.0f, -4.0f },
    { 8.0f, 0.0f, 4.0f },
    { 4.0f, 0.0f, 4.0f },
    { 0.0f, 0.0f, 4.0f }
};

//fot right hand coordinates
glm::mat4 look_at(const glm::vec3& eye, const glm::vec3& centre, const glm::vec3& up)
{
    const glm::vec3 z_axis = glm::normalize(centre - eye);
    const glm::vec3 x_axis = glm::normalize(glm::cross(z_axis, up));
    const glm::vec3 y_axis = glm::cross(x_axis, z_axis);

    glm::mat4 view_matrix;
    view_matrix[0][0] = x_axis.x;
    view_matrix[1][0] = x_axis.y;
    view_matrix[2][0] = x_axis.z;
    view_matrix[3][0] = -glm::dot(x_axis, eye);

    view_matrix[0][1] = y_axis.x;
    view_matrix[1][1] = y_axis.y;
    view_matrix[2][1] = y_axis.z;
    view_matrix[3][1] = -glm::dot(y_axis, eye);

    view_matrix[0][2] = -z_axis.x;
    view_matrix[1][2] = -z_axis.y;
    view_matrix[2][2] = -z_axis.z;
    view_matrix[3][2] = glm::dot(z_axis, eye);

    view_matrix[0][3] = 0.f;
    view_matrix[1][3] = 0.f;
    view_matrix[2][3] = 0.f;
    view_matrix[3][3] = 1.f;

    return view_matrix;
}

glm::mat4 calc_key_frame(const camera_t& key_position)
{
    return look_at(key_position.eye, key_position.centre, key_position.up);
}

key_frames_t calc_key_frames(const std::vector<camera_t>& key_positions)
{
    key_frames_t res;
    for (auto i : key_positions)
        res.push_back(calc_key_frame(i));
    return res;
}

const std::vector<camera_t> key_positions =
{ { glm::vec3(0.f, 5.f, 0.f), glm::vec3(0.f, 0.f, -4.f), glm::vec3(0.f, 1.f, 0.f) }
, { glm::vec3(3.f, 9.f, 0.f), glm::vec3(4.f, 0.f, -4.f), glm::vec3(0.f, 1.f, 0.f) }
, { glm::vec3(10.f, 1.f, 0.f), glm::vec3(8.f, 0.f, -4.f), glm::vec3(0.f, 1.f, 0.f) }

, { glm::vec3(8.f, 8.f, 0.f), glm::vec3(3.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f) }

, { glm::vec3(8.f, 8.f, 0.f), glm::vec3(8.f, 0.f, 4.f), glm::vec3(0.f, 1.f, 0.f) }
, { glm::vec3(6.f, -3.f, 0.f), glm::vec3(4.f, 0.f, 4.f), glm::vec3(0.f, 1.f, 0.f) }
, { glm::vec3(-3.f, 6.f, 0.f), glm::vec3(0.f, 0.f, 4.f), glm::vec3(0.f, 1.f, 0.f) }

, { glm::vec3(0.f, 6.f, 0.f), glm::vec3(3.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f) }
, { glm::vec3(0.f, 5.f, 0.f), glm::vec3(0.f, 0.f, -4.f), glm::vec3(0.f, 1.f, 0.f) } };
const auto key_frames = calc_key_frames(key_positions);

const intepolator_t intepolator = slerp;
const custom_func_t sqrt_func = static_cast<double(*)(double)>(sqrt);
const custom_func_t sqr_func = [](double d){ return pow(d, 2); };
size_t step = 0;
size_t curr_frame = 0;
bool game_started = false;


void draw_object(const glm::vec3& eye, const std::vector<float>& colors)
{
    glPushMatrix();
    glColor3d(colors[0], colors[1], colors[2]);
    glTranslated(eye.x, eye.y, eye.z);
    glutSolidTeapot(1);
    glPopMatrix();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < objects.size(); ++i)
    {
        draw_object(objects[i], colors[i]);
    }

    const size_t path_step = 100;

    if (curr_frame < path_step && game_started)
        ++curr_frame;
    if (step >= key_frames.size() - 1)
    {
        step = 0;
    }

    const glm::mat4 view_matrix = intepolator(key_frames[step], key_frames[step + 1], sqrt_func(static_cast<double>(curr_frame) / path_step));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view_matrix[0].data);

    glutSwapBuffers();

    glutPostRedisplay();
}

void button_event(unsigned char key, int x, int y)
{
    curr_frame = 0;
    if (game_started)
        ++step;
    game_started = true;
}

void resize(int width, int height)
{
    const float ar = (float)width / (float)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char *argv[])
{
    const GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

    const GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat high_shininess[] = { 100.0f };

    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(10, 10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Teapots invasion: press any key to move camera");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(button_event);

    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glutMainLoop();

    return EXIT_SUCCESS;
}