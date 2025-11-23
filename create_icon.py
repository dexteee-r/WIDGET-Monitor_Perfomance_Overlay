#!/usr/bin/env python3
"""
Script pour créer une icône simple pour Performance Overlay
Nécessite Pillow : pip install pillow
"""

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("ERREUR: Pillow n'est pas installé.")
    print("Installez-le avec: pip install pillow")
    exit(1)

def create_icon():
    """Crée une icône 256x256 avec un design moderne"""

    # Créer une image 256x256 avec fond transparent
    size = 256
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Fond circulaire dégradé (bleu foncé)
    center = size // 2
    radius = 110

    # Cercle de fond
    draw.ellipse(
        [center - radius, center - radius, center + radius, center + radius],
        fill=(30, 30, 35, 255),
        outline=(0, 120, 215, 255),
        width=8
    )

    # Dessiner trois barres de progression (style graphique)
    bar_width = 100
    bar_height = 16
    bar_spacing = 30
    start_x = (size - bar_width) // 2
    start_y = center - (bar_spacing + bar_height)

    colors = [
        (0, 200, 100, 255),   # Vert
        (255, 180, 0, 255),   # Orange
        (255, 70, 70, 255)    # Rouge
    ]

    fills = [0.8, 0.6, 0.4]  # Pourcentages de remplissage

    for i, (color, fill) in enumerate(zip(colors, fills)):
        y = start_y + i * bar_spacing

        # Fond de la barre
        draw.rectangle(
            [start_x, y, start_x + bar_width, y + bar_height],
            fill=(50, 50, 55, 255),
            outline=(70, 70, 75, 255),
            width=2
        )

        # Barre remplie
        fill_width = int(bar_width * fill)
        draw.rectangle(
            [start_x + 2, y + 2, start_x + fill_width - 2, y + bar_height - 2],
            fill=color
        )

    # Sauvegarder en différentes tailles pour l'icône Windows
    icon_sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    images = []

    for icon_size in icon_sizes:
        if icon_size == (256, 256):
            images.append(img)
        else:
            resized = img.resize(icon_size, Image.Resampling.LANCZOS)
            images.append(resized)

    # Sauvegarder comme .ico
    images[0].save(
        'icon.ico',
        format='ICO',
        sizes=[size for size in icon_sizes],
        append_images=images[1:]
    )

    print("✓ Icône créée avec succès : icon.ico")
    print(f"  Tailles incluses : {', '.join([f'{w}x{h}' for w, h in icon_sizes])}")

if __name__ == "__main__":
    create_icon()
