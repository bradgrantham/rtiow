#include "ppm.h"
#include "camera.h"
#include "sphere.h"
#include "lambertian.h"
#include "metal.h"
#include "dielectric.h"

namespace
{
RandomGenerator rg;

vec3 colorAt(const Ray& r, const Hittable* world, int depth)
{
    HitInfo info;
    if (world->hit(r, 0.001f, MAXFLOAT, info))
    {
        Ray scattered;
        vec3 attenuation;
        if (depth < 50 && info.material->scatter(r, info, attenuation, scattered))
        {
            return attenuation * colorAt(scattered, world, depth + 1);
        }
        return vec3(0.0f);
    }
    else
    {
        vec3 direction = r.direction();
        direction.normalize();
        float t = 0.5f * (direction.y() + 1.0f);
        return (1.0f - t) * vec3(1.0) + t * vec3(0.5f, 0.7f, 1.0f);
    }
}

void applyGamma(vec3& linearColor)
{
    const float gammaPower(1.0f / 2.2f);
    linearColor.x(pow(linearColor.x(), gammaPower));
    linearColor.y(pow(linearColor.y(), gammaPower));
    linearColor.z(pow(linearColor.z(), gammaPower));
}

} // namespace

Hittable* createWorld()
{
    int n = 500;
    Hittable** list = new Hittable*[n+1];
    list[0] = new Sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f)));
    int i = 1;
    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            float randMat = rg.getZeroToOne();
            vec3 center(a + 0.9f * rg.getZeroToOne(), 0.2f, b + 0.9f * rg.getZeroToOne());
            if ((center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
            {
                if (randMat < 0.8f) // diffuse
                {
                    list[i++] = new Sphere(center, center + vec3(0.0f, 0.5f * rg.getZeroToOne(), 0.0f),
                        0.0f, 1.0f, 0.2f, new Lambertian(vec3(rg.getZeroToOne() * rg.getZeroToOne(),
                                                              rg.getZeroToOne() * rg.getZeroToOne(),
                                                              rg.getZeroToOne() * rg.getZeroToOne())));
                }
                else if (randMat < 0.95f) // metal
                {
                    list[i++] = new Sphere(center, 0.2,
                        new Metal(vec3(0.5f * (1.0f + rg.getZeroToOne()), 0.5f * (1.0f + rg.getZeroToOne()), 0.5f * (1.0f + rg.getZeroToOne())),
                            0.5f * rg.getZeroToOne()));
                }
                else // glass
                {
                    list[i++] = new Sphere(center, 0.2f, new Dielectric(1.5f));
                }
            }
        }
    }

    list[i++] = new Sphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, new Dielectric(1.5f));
    list[i++] = new Sphere(vec3(-4.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.4f, 0.2f, 0.1f)));
    list[i++] = new Sphere(vec3(4.0f, 1.0f, 0.0f), 1.0f, new Metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));

    return new HittableList(list, i);
}

int main(int argc, char** argv)
{
    const unsigned int width(800);
    const unsigned int height(400);
    const unsigned int maxUIColor(255);
    const float maxColor(255.99f);
    const unsigned int numSamples(400);

    PPMImage ppm(width, height, maxUIColor);
    ppm.emitHeader();

    Hittable* world = createWorld();//new HittableList(list, numHittables);
    vec3 lookFrom(13.0f, 2.0f, 3.0f);
    vec3 lookAt(0.0f, 0.0f, -1.0f);
    vec3 vUp(0.0f, 1.0f, 0.0f);
    float distToFocus = (lookFrom - lookAt).length();
    float aperture(0.0f);
    Camera camera(lookFrom, lookAt, vUp, 20.0f, float(width) / float(height), aperture, distToFocus, 0.0f, 1.0f);

    for (unsigned int y = height - 1; y >= 0 && y < height; y--)
    {
        for (unsigned int x = 0; x < width; x++)
        {
            vec3 color;
            for (unsigned int s = 0; s < numSamples; s++)
            {
                float v = float(y + rg.getZeroToOne()) / float(height);
                float u = float(x + rg.getZeroToOne()) / float(width);
                Ray r = camera.getRay(u, v);
                vec3 point = r.pointAt(2.0f);
                color += colorAt(r, world, 0);
            }
            color /= float(numSamples);
            applyGamma(color);
            unsigned int ur = static_cast<unsigned int>(maxColor * color.x());
            unsigned int ug = static_cast<unsigned int>(maxColor * color.y());
            unsigned int ub = static_cast<unsigned int>(maxColor * color.z());
            ppm.emitOneColor(ur, ug, ub);
        }
    }
}
