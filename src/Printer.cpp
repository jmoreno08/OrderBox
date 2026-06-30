#include "App.h"

namespace PrinterService {
void printOrder(const Order& order) {
  Serial.println();
  Serial.println("========== ORDERBOX TICKET ==========");
  Serial.println(business.businessName.length() ? business.businessName : settings.businessName);
  Serial.print("Pedido #");
  Serial.println(order.id);

  if (order.sourceType == "table") {
    Serial.print("Mesa: ");
    Serial.println(order.table);
  } else {
    Serial.print("Mostrador #");
    Serial.println(order.counterNumber);
  }

  Serial.println("-------------------------------------");

  float total = 0;
  for (int i = 0; i < order.itemCount; i++) {
    const OrderItem& item = order.items[i];
    Serial.print(item.qty);
    Serial.print(" x ");
    Serial.print(item.name);
    Serial.print("  $");
    Serial.println(item.price * item.qty);
    total += item.price * item.qty;

    for (int j = 0; j < item.extraCount; j++) {
      Serial.print("  + ");
      Serial.print(item.extras[j].name);
      Serial.print("  $");
      Serial.println(item.extras[j].price * item.qty);
      total += item.extras[j].price * item.qty;
    }

    if (item.notes.length() > 0) {
      Serial.print("  Nota: ");
      Serial.println(item.notes);
    }
  }

  if (order.notes.length() > 0) {
    Serial.println("-------------------------------------");
    Serial.print("Obs: ");
    Serial.println(order.notes);
  }

  Serial.println("-------------------------------------");
  Serial.print("TOTAL $");
  Serial.println(total);
  Serial.println("=====================================");
  Serial.println();
}
}

void printOrderTicket(const Order& order) {
  PrinterService::printOrder(order);
}
